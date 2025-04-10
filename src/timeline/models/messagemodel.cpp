// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"

#include <QEvent>

#include <Quotient/events/encryptedevent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/stickerevent.h>
#if Quotient_VERSION_MINOR > 9 || (Quotient_VERSION_MINOR == 9 && Quotient_VERSION_PATCH > 1)
#include <Quotient/thread.h>
#endif

#include <KFormat>

#include "contentprovider.h"
#include "enums/delegatetype.h"
#include "enums/messagecomponenttype.h"
#include "eventhandler.h"
#include "events/pollevent.h"
#include "models/reactionmodel.h"
#include "neochatroommember.h"

using namespace Quotient;

std::function<bool(const Quotient::RoomEvent *)> MessageModel::m_hiddenFilter = [](const Quotient::RoomEvent *) -> bool {
    return false;
};
bool MessageModel::m_threadsEnabled = false;

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &MessageModel::newEventAdded, this, &MessageModel::createEventObjects);

    connect(this, &MessageModel::modelAboutToBeReset, this, [this]() {
        resetting = true;
    });
    connect(this, &MessageModel::modelReset, this, [this]() {
        resetting = false;
    });

    connect(this, &MessageModel::threadsEnabledChanged, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

NeoChatRoom *MessageModel::room() const
{
    return m_room;
}

void MessageModel::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }

    clearModel();

    beginResetModel();
    m_room = room;
    if (m_room != nullptr) {
        m_room->setVisible(true);
    }
    Q_EMIT roomChanged();
    endResetModel();
}

int MessageModel::timelineServerIndex() const
{
    return 0;
}

std::optional<std::reference_wrapper<const Quotient::RoomEvent>> MessageModel::getEventForIndex(QModelIndex index) const
{
    Q_UNUSED(index)
    return std::nullopt;
}

static NeochatRoomMember *emptyNeochatRoomMember = new NeochatRoomMember;

QVariant MessageModel::data(const QModelIndex &idx, int role) const
{
    if (!checkIndex(idx, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return {};
    }
    const auto row = idx.row();

    if (!m_room || row < 0 || row >= rowCount()) {
        return {};
    };

    if (m_lastReadEventIndex.row() == row) {
        switch (role) {
        case DelegateTypeRole:
            return DelegateType::ReadMarker;
        case TimeRole: {
            const QDateTime eventDate = data(index(m_lastReadEventIndex.row() + 1, 0), TimeRole).toDateTime().toLocalTime();
            static const KFormat format;
            return format.formatRelativeDateTime(eventDate, QLocale::ShortFormat);
        }
        case SpecialMarksRole:
            // Check if all the earlier events in the timeline are hidden. If so hide this.
            for (auto r = row - 1; r >= 0; --r) {
                const auto specialMark = index(r).data(SpecialMarksRole);
                if (!(specialMark == EventStatus::Hidden || specialMark == EventStatus::Replaced)) {
                    return EventStatus::Normal;
                }
            }
            return EventStatus::Hidden;
        }
        return {};
    }

    bool isPending = row < timelineServerIndex();

    const auto event = getEventForIndex(idx);
    if (!event.has_value()) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        return EventHandler::richBody(m_room, &event.value().get());
    }

    if (role == ContentModelRole) {
        if (event->get().is<EncryptedEvent>() || event->get().is<PollStartEvent>()) {
            return QVariant::fromValue<MessageContentModel *>(ContentProvider::self().contentModelForEvent(m_room, event->get().id()));
        }

        auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event.value().get());
        if (m_threadsEnabled && roomMessageEvent && roomMessageEvent->isThreaded()) {
            return QVariant::fromValue<MessageContentModel *>(ContentProvider::self().contentModelForEvent(m_room, roomMessageEvent->threadRootEventId()));
        }
        return QVariant::fromValue<MessageContentModel *>(ContentProvider::self().contentModelForEvent(m_room, &event->get()));
    }

    if (role == GenericDisplayRole) {
        return EventHandler::genericBody(m_room, &event.value().get());
    }

    if (role == DelegateTypeRole) {
        return DelegateType::typeForEvent(event.value().get());
    }

    if (role == AuthorRole) {
        QString mId;
        if (isPending) {
            mId = m_room->localMember().id();
        } else {
            mId = event.value().get().senderId();
        }

        if (!m_room->isMember(mId)) {
            return QVariant::fromValue<NeochatRoomMember *>(emptyNeochatRoomMember);
        }

        return QVariant::fromValue<NeochatRoomMember *>(m_room->qmlSafeMember(mId));
    }

    if (role == HighlightRole) {
        return EventHandler::isHighlighted(m_room, &event.value().get());
    }

    if (role == SpecialMarksRole) {
        if (isPending) {
            // A pending event with an m.new_content key will be merged into the
            // original event so don't show.
            if (event.value().get().contentJson().contains("m.new_content"_L1)) {
                return EventStatus::Hidden;
            }
            const auto pendingIt = m_room->findPendingEvent(event->get().transactionId());
            if (pendingIt == m_room->pendingEvents().end()) {
                return EventStatus::Hidden;
            }
            return pendingIt->deliveryStatus();
        }

        if (EventHandler::isHidden(m_room, &event.value().get(), m_hiddenFilter)) {
            return EventStatus::Hidden;
        }

        auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event.value().get());
#if Quotient_VERSION_MINOR > 9 || (Quotient_VERSION_MINOR == 9 && Quotient_VERSION_PATCH > 1)
        if (roomMessageEvent && (roomMessageEvent->isThreaded() || m_room->threads().contains(event.value().get().id()))) {
            const auto &thread = m_room->threads().value(roomMessageEvent->isThreaded() ? roomMessageEvent->threadRootEventId() : event.value().get().id());
            if (thread.latestEventId != event.value().get().id()) {
                return EventStatus::Hidden;
            }
        }
#else
        if (roomMessageEvent && roomMessageEvent->isThreaded() && roomMessageEvent->threadRootEventId() != event.value().get().id() && m_threadsEnabled) {
            return EventStatus::Hidden;
        }
#endif

        return EventStatus::Normal;
    }

    if (role == EventIdRole) {
        return event.value().get().displayId();
    }

    if (role == ProgressInfoRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&event.value().get())) {
            if (e->has<EventContent::FileContent>() || e->has<EventContent::ImageContent>() || e->has<EventContent::VideoContent>()
                || e->has<EventContent::AudioContent>()) {
                return QVariant::fromValue(m_room->cachedFileTransferInfo(&event.value().get()));
            }
        }
        if (eventCast<const StickerEvent>(&event.value().get())) {
            return QVariant::fromValue(m_room->cachedFileTransferInfo(&event.value().get()));
        }
        return {};
    }

    if (role == TimeRole) {
        return EventHandler::time(m_room, &event.value().get(), isPending);
    }

    if (role == SectionRole) {
        return EventHandler::timeString(m_room, &event.value().get(), true, QLocale::ShortFormat, isPending);
    }

    if (role == IsThreadedRole) {
        if (!m_threadsEnabled) {
            return false;
        }
        if (auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event.value().get())) {
            return roomMessageEvent->isThreaded();
        }
        return {};
    }

    if (role == ThreadRootRole) {
        auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event.value().get());
        if (roomMessageEvent && roomMessageEvent->isThreaded()) {
            return roomMessageEvent->threadRootEventId();
        }
        return {};
    }

    if (role == IsPollRole) {
        return event->get().is<PollStartEvent>();
    }

    if (role == ShowSectionRole) {
        for (auto r = row + 1; r < rowCount(); ++r) {
            auto i = index(r);
            // Note !itemData(i).empty() is a check for instances where rows have been removed, e.g. when the read marker is moved.
            // While the row is removed the subsequent row indexes are not changed so we need to skip over the removed index.
            // See - https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
            if (data(i, SpecialMarksRole) != EventStatus::Hidden && !itemData(i).empty()) {
                const auto day = data(idx, TimeRole).toDateTime().toLocalTime().date().dayOfYear();
                const auto previousEventDay = data(i, TimeRole).toDateTime().toLocalTime().date().dayOfYear();
                return day != previousEventDay;
            }
        }

        return false;
    }

    if (role == ReadMarkersRole) {
        if (m_readMarkerModels.contains(event.value().get().id())) {
            return QVariant::fromValue<ReadMarkerModel *>(m_readMarkerModels[event.value().get().id()].get());
        } else {
            return QVariantList();
        }
    }

    if (role == ShowReadMarkersRole) {
        return m_readMarkerModels.contains(event.value().get().id());
    }

    if (role == VerifiedRole) {
        if (event.value().get().originalEvent()) {
            auto encrypted = dynamic_cast<const EncryptedEvent *>(event.value().get().originalEvent());
            Q_ASSERT(encrypted);
            return m_room->connection()->isVerifiedSession(encrypted->sessionId().toLatin1());
        }
        return false;
    }

    if (role == AuthorDisplayNameRole) {
        return EventHandler::authorDisplayName(m_room, &event.value().get(), isPending);
    }

    if (role == IsRedactedRole) {
        return event.value().get().isRedacted();
    }

    if (role == IsPendingRole) {
        return row < static_cast<int>(m_room->pendingEvents().size());
    }

    if (role == MediaInfoRole) {
        return EventHandler::mediaInfo(m_room, &event.value().get());
    }

    if (role == IsEditableRole) {
        return MessageComponentType::typeForEvent(event.value().get()) == MessageComponentType::Text
            && event.value().get().senderId() == m_room->localMember().id();
    }

    if (role == ShowAuthorRole) {
        return true;
    }

    return {};
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[DelegateTypeRole] = "delegateType";
    roles[EventIdRole] = "eventId";
    roles[TimeRole] = "time";
    roles[SectionRole] = "section";
    roles[AuthorRole] = "author";
    roles[HighlightRole] = "isHighlighted";
    roles[SpecialMarksRole] = "marks";
    roles[ProgressInfoRole] = "progressInfo";
    roles[IsThreadedRole] = "isThreaded";
    roles[ThreadRootRole] = "threadRoot";
    roles[IsPollRole] = "isPoll";
    roles[ShowSectionRole] = "showSection";
    roles[ReadMarkersRole] = "readMarkers";
    roles[ShowReadMarkersRole] = "showReadMarkers";
    roles[VerifiedRole] = "verified";
    roles[AuthorDisplayNameRole] = "authorDisplayName";
    roles[IsRedactedRole] = "isRedacted";
    roles[GenericDisplayRole] = "genericDisplay";
    roles[IsPendingRole] = "isPending";
    roles[ContentModelRole] = "contentModel";
    roles[MediaInfoRole] = "mediaInfo";
    roles[IsEditableRole] = "isEditable";
    roles[ShowAuthorRole] = "showAuthor";
    return roles;
}

int MessageModel::eventIdToRow(const QString &eventID) const
{
    if (m_room == nullptr) {
        return -1;
    }

    const auto it = m_room->findInTimeline(eventID);
    if (it == m_room->historyEdge()) {
        // qWarning() << "Trying to find inexistent event:" << eventID;
        return -1;
    }
    return it - m_room->messageEvents().rbegin() + timelineServerIndex();
}

void MessageModel::fullEventRefresh(int row)
{
    auto roles = roleNames().keys();
    // The author of an event never changes so should only be updated when a member
    // changed signal is emitted.
    // This also avoids any race conditions where a member is updating and this refresh
    // tries to access a member event that has already been deleted.
    roles.removeAll(AuthorRole);
    refreshEventRoles(row, roles);
}

void MessageModel::refreshEventRoles(int row, const QList<int> &roles)
{
    const auto idx = index(row);
    Q_EMIT dataChanged(idx, idx, roles);
}

int MessageModel::refreshEventRoles(const QString &id, const QList<int> &roles)
{
    // On 64-bit platforms, difference_type for std containers is long long
    // but Qt uses int throughout its interfaces; hence casting to int below.
    int row = -1;
    // First try pendingEvents because it is almost always very short.
    const auto pendingIt = m_room->findPendingEvent(id);
    if (pendingIt != m_room->pendingEvents().end()) {
        row = int(pendingIt - m_room->pendingEvents().begin());
    } else {
        const auto timelineIt = m_room->findInTimeline(id);
        if (timelineIt == m_room->historyEdge()) {
            return -1;
        }
        row = int(timelineIt - m_room->messageEvents().rbegin()) + timelineServerIndex();
        if (data(index(row, 0), DelegateTypeRole).toInt() == DelegateType::ReadMarker || data(index(row, 0), DelegateTypeRole).toInt() == DelegateType::Other) {
            row++;
        }
    }
    refreshEventRoles(row, roles);
    return row;
}

void MessageModel::refreshLastUserEvents(int baseTimelineRow)
{
    if (!m_room || m_room->timelineSize() <= baseTimelineRow) {
        return;
    }

    const auto &timelineBottom = m_room->messageEvents().rbegin();
    const auto &lastSender = (*(timelineBottom + baseTimelineRow))->senderId();
    const auto limit = timelineBottom + std::min(baseTimelineRow + 10, m_room->timelineSize());
    for (auto it = timelineBottom + std::max(baseTimelineRow - 10, 0); it != limit; ++it) {
        if ((*it)->senderId() == lastSender) {
            fullEventRefresh(it - timelineBottom);
        }
    }
}

void MessageModel::createEventObjects(const Quotient::RoomEvent *event)
{
    if (event == nullptr) {
        return;
    }

    auto eventId = event->id();
    auto senderId = event->senderId();
    if (eventId.isEmpty()) {
        eventId = event->transactionId();
    }
    // A pending event might not have a sender ID set yet but in that case it must
    // be the local member.
    if (senderId.isEmpty()) {
        senderId = m_room->localMember().id();
    }

    // ReadMarkerModel handles updates to add and remove markers, we only need to
    // handle adding and removing whole models here.
    if (m_readMarkerModels.contains(eventId)) {
        // If a model already exists but now has no reactions remove it
        if (m_readMarkerModels[eventId]->rowCount() <= 0) {
            m_readMarkerModels.remove(eventId);
            if (!resetting) {
                refreshEventRoles(eventId, {ReadMarkersRole, ShowReadMarkersRole});
            }
        }
    } else {
        auto memberIds = m_room->userIdsAtEvent(eventId);
        memberIds.remove(m_room->localMember().id());
        if (memberIds.size() > 0) {
            // If a model doesn't exist and there are reactions add it.
            auto newModel = QSharedPointer<ReadMarkerModel>(new ReadMarkerModel(eventId, m_room));
            if (newModel->rowCount() > 0) {
                m_readMarkerModels[eventId] = newModel;
                if (!resetting) {
                    refreshEventRoles(eventId, {ReadMarkersRole, ShowReadMarkersRole});
                }
            }
        }
    }
}

void MessageModel::clearModel()
{
    if (m_room) {
        const auto oldRoom = m_room;

        // HACK: Reset the model to a null room first to make sure QML dismantles
        // last room's objects before the room is actually changed
        beginResetModel();
        m_room->disconnect(this);
        m_room = nullptr;
        endResetModel();

        // Because we don't want any of the object deleted before the model is cleared.
        oldRoom->setVisible(false);
    }

    // Don't clear the member objects until the model has been fully reset and all
    // refs cleared.
    clearEventObjects();
}

void MessageModel::clearEventObjects()
{
    m_readMarkerModels.clear();
}

bool MessageModel::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole, ReadMarkersRole});
    }
    return QObject::event(event);
}

void MessageModel::setHiddenFilter(std::function<bool(const Quotient::RoomEvent *)> hiddenFilter)
{
    MessageModel::m_hiddenFilter = hiddenFilter;
}

void MessageModel::setThreadsEnabled(bool enableThreads)
{
    MessageModel::m_threadsEnabled = enableThreads;
}

#include "moc_messagemodel.cpp"
