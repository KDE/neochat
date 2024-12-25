// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pinnedmessagesmodel.h"

#include "enums/delegatetype.h"
#include "eventhandler.h"
#include "models/messagecontentmodel.h"
#include "neochatroom.h"

#include <QGuiApplication>

#include <KLocalizedString>

using namespace Quotient;

PinnedMessagesModel::PinnedMessagesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant PinnedMessagesModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    const auto &event = *m_pinnedEvents[row];

    switch (role) {
    case AuthorRole:
        return QVariant::fromValue<NeochatRoomMember *>(new NeochatRoomMember(m_room, event.senderId()));
    case ShowReactionsRole:
        return false;
    case ShowReadMarkersRole:
        return false;
    case IsPendingRole:
        return false;
    case HighlightRole:
        return EventHandler::isHighlighted(m_room, &event);
    case EventIdRole:
        return event.displayId();
    case IsThreadedRole:
        if (auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event)) {
            return roomMessageEvent->isThreaded();
        }
        return {};
    case ThreadRootRole:
        if (auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event); roomMessageEvent->isThreaded()) {
            return roomMessageEvent->threadRootEventId();
        }
        return {};
    case ContentModelRole: {
        if (!event.isStateEvent()) {
            return QVariant::fromValue<MessageContentModel *>(new MessageContentModel(m_room, event.id()));
        }
        if (event.isStateEvent()) {
            if (event.matrixType() == u"org.matrix.msc3672.beacon_info"_s) {
                return QVariant::fromValue<MessageContentModel *>(new MessageContentModel(m_room, event.id()));
            }
        }
        return {};
    }
    case IsEditableRole: {
        return false;
    }
    }
    return DelegateType::Message;
}

int PinnedMessagesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_pinnedEvents.size();
}

QHash<int, QByteArray> PinnedMessagesModel::roleNames() const
{
    return {
        {DelegateTypeRole, "delegateType"},
        {AuthorRole, "author"},
        {ShowSectionRole, "showSection"},
        {SectionRole, "section"},
        {EventIdRole, "eventId"},
        {ExcessReadMarkersRole, "excessReadMarkers"},
        {HighlightRole, "isHighlighted"},
        {ReadMarkersString, "readMarkersString"},
        {VerifiedRole, "verified"},
        {ShowReactionsRole, "showReactions"},
        {ReactionRole, "reaction"},
        {ReadMarkersRole, "readMarkers"},
        {IsPendingRole, "isPending"},
        {ShowReadMarkersRole, "showReadMarkers"},
        {IsThreadedRole, "isThreaded"},
        {ThreadRootRole, "threadRoot"},
        {ContentModelRole, "contentModel"},
        {IsEditableRole, "isEditable"},
    };
}

NeoChatRoom *PinnedMessagesModel::room() const
{
    return m_room;
}

void PinnedMessagesModel::setRoom(NeoChatRoom *room)
{
    if (m_room) {
        disconnect(m_room, nullptr, this, nullptr);
    }
    m_room = room;
    Q_EMIT roomChanged();

    const auto events = m_room->pinnedEventIds();

    for (auto event : events) {
        auto job = m_room->connection()->callApi<GetOneRoomEventJob>(m_room->id(), event);
        connect(job, &BaseJob::success, this, [this, job] {
            beginInsertRows({}, m_pinnedEvents.size(), m_pinnedEvents.size());
            m_pinnedEvents.push_back(std::move(fromJson<event_ptr_tt<RoomEvent>>(job->jsonData())));
            endInsertRows();
        });
        connect(job, &BaseJob::failure, this, [this, job] {
            if (job->error() == BaseJob::NotFound) {
                // Q_EMIT extraEventNotFound(eventId);
            }
        });
    }
}

bool PinnedMessagesModel::loading() const
{
    return m_loading;
}

bool PinnedMessagesModel::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole, ReadMarkersRole});
    }
    return QObject::event(event);
}

void PinnedMessagesModel::setLoading(bool loading)
{
    m_loading = loading;
    Q_EMIT loadingChanged();
}

#include "moc_pinnedmessagesmodel.cpp"
