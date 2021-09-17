// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "messageeventmodel.h"

#include "neochatconfig.h"
#include <connection.h>
#include <events/reactionevent.h>
#include <events/redactionevent.h>
#include <events/roomavatarevent.h>
#include <events/roommemberevent.h>
#include <events/simplestateevents.h>
#include <user.h>

#include "stickerevent.h"

#include <QDebug>
#include <QQmlEngine> // for qmlRegisterType()
#include <QTimeZone>

#include <KFormat>
#include <KLocalizedString>

#include "utils.h"

QHash<int, QByteArray> MessageEventModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[EventTypeRole] = "eventType";
    roles[MessageRole] = "message";
    roles[EventIdRole] = "eventId";
    roles[TimeRole] = "time";
    roles[SectionRole] = "section";
    roles[AuthorRole] = "author";
    roles[ContentRole] = "content";
    roles[ContentTypeRole] = "contentType";
    roles[HighlightRole] = "isHighlighted";
    roles[SpecialMarksRole] = "marks";
    roles[LongOperationRole] = "progressInfo";
    roles[FileMimetypeIcon] = "fileMimetypeIcon";
    roles[AnnotationRole] = "annotation";
    roles[EventResolvedTypeRole] = "eventResolvedType";
    roles[ReplyRole] = "reply";
    roles[UserMarkerRole] = "userMarker";
    roles[ShowAuthorRole] = "showAuthor";
    roles[ShowSectionRole] = "showSection";
    roles[ReactionRole] = "reaction";
    roles[IsEditedRole] = "isEdited";
    roles[FormattedBodyRole] = "formattedBody";
    return roles;
}

MessageEventModel::MessageEventModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentRoom(nullptr)
{
    using namespace Quotient;
    qmlRegisterAnonymousType<FileTransferInfo>("org.kde.neochat", 1);
    qRegisterMetaType<FileTransferInfo>();

    QTimer::singleShot(0, this, [=]() {
        if (!m_currentRoom) {
            return;
        }
        m_currentRoom->getPreviousContent(50);
        connect(this, &QAbstractListModel::rowsInserted, this, [=]() {
            if (m_currentRoom->readMarkerEventId().isEmpty()) {
                return;
            }
            const auto it = m_currentRoom->findInTimeline(m_currentRoom->readMarkerEventId());
            if (it == m_currentRoom->historyEdge()) {
                m_currentRoom->getPreviousContent(50);
            }
        });
    });
}

MessageEventModel::~MessageEventModel() = default;

void MessageEventModel::setRoom(NeoChatRoom *room)
{
    if (room == m_currentRoom) {
        return;
    }

    beginResetModel();
    if (m_currentRoom) {
        m_currentRoom->disconnect(this);
    }

    m_currentRoom = room;
    if (room) {
        m_lastReadEventIndex = QPersistentModelIndex(QModelIndex());
        room->setDisplayed();
        if (m_currentRoom->timelineSize() < 10) {
            room->getPreviousContent(50);
        }
        lastReadEventId = room->readMarkerEventId();

        using namespace Quotient;
        connect(m_currentRoom, &Room::aboutToAddNewMessages, this, [=](RoomEventsRange events) {
            if (NeoChatConfig::self()->showFancyEffects()) {
                for (auto &event : events) {
                    RoomMessageEvent *message = dynamic_cast<RoomMessageEvent *>(event.get());
                    if (message) {
                        QString planBody = message->plainBody();
                        // snowflake
                        const QString snowlakeEmoji = QString::fromUtf8("\xE2\x9D\x84");
                        if (planBody.contains(snowlakeEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("snowflake"));
                        }
                        // fireworks
                        const QString fireworksEmoji = QString::fromUtf8("\xF0\x9F\x8E\x86");
                        if (planBody.contains(fireworksEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("fireworks"));
                        }
                        // sparkler
                        const QString sparklerEmoji = QString::fromUtf8("\xF0\x9F\x8E\x87");
                        if (planBody.contains(sparklerEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("fireworks"));
                        }
                        // party pooper
                        const QString partyEmoji = QString::fromUtf8("\xF0\x9F\x8E\x89");
                        if (planBody.contains(partyEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("confetti"));
                        }
                        // confetti ball
                        const QString confettiEmoji = QString::fromUtf8("\xF0\x9F\x8E\x8A");
                        if (planBody.contains(confettiEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("confetti"));
                        }
                    }
                }
            }
            beginInsertRows({}, timelineBaseIndex(), timelineBaseIndex() + int(events.size()) - 1);
        });
        connect(m_currentRoom, &Room::aboutToAddHistoricalMessages, this, [=](RoomEventsRange events) {
            if (rowCount() > 0) {
                rowBelowInserted = rowCount() - 1; // See #312
            }
            beginInsertRows({}, rowCount(), rowCount() + int(events.size()) - 1);
        });
        connect(m_currentRoom, &Room::addedMessages, this, [=](int lowest, int biggest) {
            endInsertRows();
            if (!m_lastReadEventIndex.isValid()) {
                // no read marker, so see if we need to create one.
                moveReadMarker(m_currentRoom->readMarkerEventId());
            }
            if (biggest < m_currentRoom->maxTimelineIndex()) {
                auto rowBelowInserted = m_currentRoom->maxTimelineIndex() - biggest + timelineBaseIndex() - 1;
                refreshEventRoles(rowBelowInserted, {ShowAuthorRole});
            }
            for (auto i = m_currentRoom->maxTimelineIndex() - biggest; i <= m_currentRoom->maxTimelineIndex() - lowest; ++i) {
                refreshLastUserEvents(i);
            }
        });
        connect(m_currentRoom, &Room::pendingEventAboutToAdd, this, [this] {
            beginInsertRows({}, 0, 0);
        });
        connect(m_currentRoom, &Room::pendingEventAdded, this, &MessageEventModel::endInsertRows);
        connect(m_currentRoom, &Room::pendingEventAboutToMerge, this, [this](RoomEvent *, int i) {
            if (i == 0) {
                return; // No need to move anything, just refresh
            }

            movingEvent = true;
            // Reverse i because row 0 is bottommost in the model
            const auto row = timelineBaseIndex() - i - 1;
            beginMoveRows({}, row, row, {}, timelineBaseIndex());
        });
        connect(m_currentRoom, &Room::pendingEventMerged, this, [this] {
            if (movingEvent) {
                endMoveRows();
                movingEvent = false;
            }
            refreshRow(timelineBaseIndex()); // Refresh the looks
            refreshLastUserEvents(0);
            if (timelineBaseIndex() > 0) { // Refresh below, see #312
                refreshEventRoles(timelineBaseIndex() - 1, {ShowAuthorRole});
            }
        });
        connect(m_currentRoom, &Room::pendingEventChanged, this, &MessageEventModel::refreshRow);
        connect(m_currentRoom, &Room::pendingEventAboutToDiscard, this, [this](int i) {
            beginRemoveRows({}, i, i);
        });
        connect(m_currentRoom, &Room::pendingEventDiscarded, this, &MessageEventModel::endRemoveRows);
        connect(m_currentRoom, &Room::readMarkerMoved, this, [=](const QString &fromEventId, const QString &toEventId) {
            Q_UNUSED(fromEventId);
            moveReadMarker(toEventId);
        });
        connect(m_currentRoom, &Room::replacedEvent, this, [this](const RoomEvent *newEvent) {
            refreshLastUserEvents(refreshEvent(newEvent->id()) - timelineBaseIndex());
        });
        connect(m_currentRoom, &Room::updatedEvent, this, [this](const QString &eventId) {
            if (eventId.isEmpty()) { // How did we get here?
                return;
            }
            refreshEventRoles(eventId, {ReactionRole, Qt::DisplayRole});
        });
        connect(m_currentRoom, &Room::fileTransferProgress, this, &MessageEventModel::refreshEvent);
        connect(m_currentRoom, &Room::fileTransferCompleted, this, &MessageEventModel::refreshEvent);
        connect(m_currentRoom, &Room::fileTransferFailed, this, &MessageEventModel::refreshEvent);
#ifndef QUOTIENT_07
        connect(m_currentRoom, &Room::fileTransferCancelled, this, &MessageEventModel::refreshEvent);
#endif
        connect(m_currentRoom->connection(), &Connection::ignoredUsersListChanged, this, [=] {
            beginResetModel();
            endResetModel();
        });
        qDebug() << "Connected to room" << room->id() << "as" << room->localUser()->id();
    } else {
        lastReadEventId.clear();
    }
    endResetModel();
}

int MessageEventModel::refreshEvent(const QString &eventId)
{
    return refreshEventRoles(eventId);
}

void MessageEventModel::refreshRow(int row)
{
    refreshEventRoles(row);
}

int MessageEventModel::timelineBaseIndex() const
{
    return m_currentRoom ? int(m_currentRoom->pendingEvents().size()) : 0;
}

void MessageEventModel::refreshEventRoles(int row, const QVector<int> &roles)
{
    const auto idx = index(row);
    Q_EMIT dataChanged(idx, idx, roles);
}

void MessageEventModel::moveReadMarker(const QString &toEventId)
{
    const auto timelineIt = m_currentRoom->findInTimeline(toEventId);
    if (timelineIt == m_currentRoom->historyEdge()) {
        return;
    }
    int newRow = int(timelineIt - m_currentRoom->messageEvents().rbegin()) + timelineBaseIndex();

    if (!m_lastReadEventIndex.isValid()) {
        // Not valid index means we don't display any marker yet, in this case
        // we create the new index and insert the row in case the read marker
        // need to be displayed.
        if (newRow > timelineBaseIndex()) {
            // The user didn't read all the messages yet.
            beginInsertRows({}, newRow, newRow);
            m_lastReadEventIndex = QPersistentModelIndex(index(newRow, 0));
            endInsertRows();
            return;
        }
        // The user read all the messages and we didn't display any read marker yet
        // => do nothing
        return;
    }
    if (newRow <= timelineBaseIndex()) {
        // The user read all the messages => remove read marker
        beginRemoveRows({}, m_lastReadEventIndex.row(), m_lastReadEventIndex.row());
        m_lastReadEventIndex = QModelIndex();
        endRemoveRows();
        return;
    }

    // The user didn't read all the messages yet but moved the reader marker.
    beginMoveRows({}, m_lastReadEventIndex.row(), m_lastReadEventIndex.row(), {}, newRow);
    m_lastReadEventIndex = QPersistentModelIndex(index(newRow, 0));
    endMoveRows();
}

int MessageEventModel::refreshEventRoles(const QString &id, const QVector<int> &roles)
{
    // On 64-bit platforms, difference_type for std containers is long long
    // but Qt uses int throughout its interfaces; hence casting to int below.
    int row = -1;
    // First try pendingEvents because it is almost always very short.
    const auto pendingIt = m_currentRoom->findPendingEvent(id);
    if (pendingIt != m_currentRoom->pendingEvents().end()) {
        row = int(pendingIt - m_currentRoom->pendingEvents().begin());
    } else {
        const auto timelineIt = m_currentRoom->findInTimeline(id);
        if (timelineIt == m_currentRoom->historyEdge()) {
            qWarning() << "Trying to refresh inexistent event:" << id;
            return -1;
        }
        row = int(timelineIt - m_currentRoom->messageEvents().rbegin()) + timelineBaseIndex();
    }
    refreshEventRoles(row, roles);
    return row;
}

inline bool hasValidTimestamp(const Quotient::TimelineItem &ti)
{
    return ti->originTimestamp().isValid();
}

QDateTime MessageEventModel::makeMessageTimestamp(const Quotient::Room::rev_iter_t &baseIt) const
{
    const auto &timeline = m_currentRoom->messageEvents();
    auto ts = baseIt->event()->originTimestamp();
    if (ts.isValid()) {
        return ts;
    }

    // The event is most likely redacted or just invalid.
    // Look for the nearest date around and slap zero time to it.
    using Quotient::TimelineItem;
    auto rit = std::find_if(baseIt, timeline.rend(), hasValidTimestamp);
    if (rit != timeline.rend()) {
        return {rit->event()->originTimestamp().date(), {0, 0}, Qt::LocalTime};
    };
    auto it = std::find_if(baseIt.base(), timeline.end(), hasValidTimestamp);
    if (it != timeline.end()) {
        return {it->event()->originTimestamp().date(), {0, 0}, Qt::LocalTime};
    };

    // What kind of room is that?..
    qCritical() << "No valid timestamps in the room timeline!";
    return {};
}

QString MessageEventModel::renderDate(const QDateTime &timestamp)
{
    auto date = timestamp.toLocalTime().date();
    if (date == QDate::currentDate()) {
        return i18n("Today");
    }
    if (date == QDate::currentDate().addDays(-1)) {
        return i18n("Yesterday");
    }
    if (date == QDate::currentDate().addDays(-2)) {
        return i18n("The day before yesterday");
    }
    if (date > QDate::currentDate().addDays(-7)) {
        return date.toString("dddd");
    }

    return QLocale::system().toString(date, QLocale::ShortFormat);
}

void MessageEventModel::refreshLastUserEvents(int baseTimelineRow)
{
    if (!m_currentRoom || m_currentRoom->timelineSize() <= baseTimelineRow) {
        return;
    }

    const auto &timelineBottom = m_currentRoom->messageEvents().rbegin();
    const auto &lastSender = (*(timelineBottom + baseTimelineRow))->senderId();
    const auto limit = timelineBottom + std::min(baseTimelineRow + 10, m_currentRoom->timelineSize());
    for (auto it = timelineBottom + std::max(baseTimelineRow - 10, 0); it != limit; ++it) {
        if ((*it)->senderId() == lastSender) {
            auto idx = index(it - timelineBottom);
            Q_EMIT dataChanged(idx, idx);
        }
    }
}

int MessageEventModel::rowCount(const QModelIndex &parent) const
{
    if (!m_currentRoom || parent.isValid()) {
        return 0;
    }

    const auto firstIt = m_currentRoom->messageEvents().crbegin();
    if (firstIt != m_currentRoom->messageEvents().crend()) {
        const auto &firstEvt = **firstIt;
        return m_currentRoom->timelineSize() + (lastReadEventId != firstEvt.id() ? 1 : 0);
    } else {
        return m_currentRoom->timelineSize();
    }
}

bool MessageEventModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_currentRoom && !m_currentRoom->eventsHistoryJob() && !m_currentRoom->allHistoryLoaded();
}

void MessageEventModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (m_currentRoom) {
        m_currentRoom->getPreviousContent(20);
    }
}

inline QVariantMap userAtEvent(NeoChatUser *user, NeoChatRoom *room, const RoomEvent &evt)
{
    Q_UNUSED(evt)
    return QVariantMap{
        {"isLocalUser", user->id() == room->localUser()->id()},
        {"id", user->id()},
        {"avatarMediaId", user->avatarMediaId(room)},
        {"avatarUrl", user->avatarUrl(room)},
        {"displayName", user->displayname(room)},
        {"display", user->name()},
        {"color", user->color()},
        {"object", QVariant::fromValue(user)},
    };
}

QVariant MessageEventModel::data(const QModelIndex &idx, int role) const
{
    const auto row = idx.row();

    if (!m_currentRoom || row < 0 || row >= int(m_currentRoom->pendingEvents().size()) + m_currentRoom->timelineSize()) {
        return {};
    };

    bool isPending = row < timelineBaseIndex();

    if (m_lastReadEventIndex.row() == row) {
        switch (role) {
        case EventTypeRole:
            return QStringLiteral("readMarker");
        case TimeRole: {
            const QDateTime eventDate = data(index(m_lastReadEventIndex.row() + 1, 0), TimeRole).toDateTime();
            const KFormat format;
            return format.formatRelativeDateTime(eventDate, QLocale::ShortFormat);
        }
        case SpecialMarksRole:
            return EventStatus::Hidden;
        }
        return {};
    }

    const auto timelineIt = m_currentRoom->messageEvents().crbegin()
        + std::max(0, row - timelineBaseIndex() - (m_lastReadEventIndex.isValid() && m_lastReadEventIndex.row() < row ? 1 : 0));
    const auto pendingIt = m_currentRoom->pendingEvents().crbegin() + std::min(row, timelineBaseIndex());
    const auto &evt = isPending ? **pendingIt : **timelineIt;

    if (role == Qt::DisplayRole) {
        if (evt.isRedacted()) {
            auto reason = evt.redactedBecause()->reason();
            return (reason.isEmpty()) ? i18n("<i>[This message was deleted]</i>")
                                      : i18n("<i>[This message was deleted: %1]</i>", evt.redactedBecause()->reason());
        }

        return m_currentRoom->eventToString(evt, Qt::RichText);
    }

    if (role == FormattedBodyRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            if (e->hasTextContent() && e->mimeType().name() != "text/plain") {
                return static_cast<const Quotient::EventContent::TextContent *>(e->content())->body;
            }
        }

        return {};
    }

    if (role == MessageRole) {
        return m_currentRoom->eventToString(evt);
    }

    if (role == Qt::ToolTipRole) {
        return evt.originalJson();
    }

    if (role == EventTypeRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            switch (e->msgtype()) {
            case MessageEventType::Emote:
                return "emote";
            case MessageEventType::Notice:
                return "notice";
            case MessageEventType::Image:
                return "image";
            case MessageEventType::Audio:
                return "audio";
            case MessageEventType::Video:
                return "video";
            default:
                break;
            }
            if (e->hasFileContent()) {
                return "file";
            }

            return "message";
        }
        if (is<const StickerEvent>(evt)) {
            return "sticker";
        }
        if (evt.isStateEvent()) {
            return "state";
        }
        if (is<const EncryptedEvent>(evt)) {
            return "encrypted";
        }

        return "other";
    }

    if (role == EventResolvedTypeRole) {
        return EventTypeRegistry::getMatrixType(evt.type());
    }

    if (role == AuthorRole) {
        auto author = static_cast<NeoChatUser *>(isPending ? m_currentRoom->localUser() : m_currentRoom->user(evt.senderId()));
        return userAtEvent(author, m_currentRoom, evt);
    }

    if (role == ContentTypeRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            const auto &contentType = e->mimeType().name();
            return contentType == "text/plain" ? QStringLiteral("text/html") : contentType;
        }
        return QStringLiteral("text/plain");
    }

    if (role == ContentRole) {
        if (evt.isRedacted()) {
            auto reason = evt.redactedBecause()->reason();
            return (reason.isEmpty()) ? i18n("[REDACTED]") : i18n("[REDACTED: %1]").arg(evt.redactedBecause()->reason());
        }

        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            // Cannot use e.contentJson() here because some
            // EventContent classes inject values into the copy of the
            // content JSON stored in EventContent::Base
            return e->hasFileContent() ? QVariant::fromValue(e->content()->originalJson) : QVariant();
        };

        if (auto e = eventCast<const StickerEvent>(&evt)) {
            return QVariant::fromValue(e->image().originalJson);
        }
    }

    if (role == HighlightRole) {
        return m_currentRoom->isEventHighlighted(&evt);
    }

    if (role == FileMimetypeIcon) {
        auto e = eventCast<const RoomMessageEvent>(&evt);
        if (!e || !e->hasFileContent()) {
            return QVariant();
        }

        return e->content()->fileInfo()->mimeType.iconName();
    }

    if (role == SpecialMarksRole) {
        if (isPending) {
            return pendingIt->deliveryStatus();
        }

        auto *memberEvent = timelineIt->viewAs<RoomMemberEvent>();
        if (memberEvent) {
            if ((memberEvent->isJoin() || memberEvent->isLeave()) && !NeoChatConfig::self()->showLeaveJoinEvent()) {
                return EventStatus::Hidden;
            }
        }

        // isReplacement?
        if (auto e = eventCast<const RoomMessageEvent>(&evt))
            if (!e->replacedEvent().isEmpty())
                return EventStatus::Hidden;

        if (is<RedactionEvent>(evt) || is<ReactionEvent>(evt)) {
            return EventStatus::Hidden;
        }

        if (evt.isStateEvent() && static_cast<const StateEventBase &>(evt).repeatsState()) {
            return EventStatus::Hidden;
        }

        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            if (!e->replacedEvent().isEmpty() && e->replacedEvent() != e->id()) {
                return EventStatus::Hidden;
            }
        }

        if (m_currentRoom->connection()->isIgnored(m_currentRoom->user(evt.senderId()))) {
            return EventStatus::Hidden;
        }

        return EventStatus::Normal;
    }

    if (role == IsEditedRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            return !e->unsignedJson().isEmpty() && e->unsignedJson().contains("m.relations")
                && e->unsignedJson()["m.relations"].toObject().contains("m.replace");
        }
        return false;
    }

    if (role == EventIdRole) {
        return !evt.id().isEmpty() ? evt.id() : evt.transactionId();
    }

    if (role == LongOperationRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            if (e->hasFileContent()) {
                return QVariant::fromValue(m_currentRoom->fileTransferInfo(e->id()));
            }
        }
        if (auto e = eventCast<const StickerEvent>(&evt)) {
            return QVariant::fromValue(m_currentRoom->fileTransferInfo(e->id()));
        }
    }

    if (role == AnnotationRole) {
        if (isPending) {
            return pendingIt->annotation();
        }
    }

    if (role == TimeRole || role == SectionRole) {
        auto ts = isPending ? pendingIt->lastUpdated() : makeMessageTimestamp(timelineIt);
        return role == TimeRole ? QVariant(ts) : renderDate(ts);
    }

    if (role == UserMarkerRole) {
        QVariantList variantList;
        const auto users = m_currentRoom->usersAtEventId(evt.id());
        for (User *user : users) {
            if (user == m_currentRoom->localUser()) {
                continue;
            }
            variantList.append(QVariant::fromValue(user));
        }
        return variantList;
    }

    if (role == ReplyRole) {
        const QString &replyEventId = evt.contentJson()["m.relates_to"].toObject()["m.in_reply_to"].toObject()["event_id"].toString();
        if (replyEventId.isEmpty()) {
            return {};
        };
        const auto replyIt = m_currentRoom->findInTimeline(replyEventId);
        if (replyIt == m_currentRoom->historyEdge()) {
            return {};
        };
        const auto &replyEvt = **replyIt;

        QString type;
        if (auto e = eventCast<const RoomMessageEvent>(&replyEvt)) {
            switch (e->msgtype()) {
            case MessageEventType::Emote:
                type = "emote";
                break;
            case MessageEventType::Notice:
                type = "notice";
                break;
            case MessageEventType::Image:
                type = "image";
                break;
            case MessageEventType::Audio:
                type = "audio";
                break;
            case MessageEventType::Video:
                type = "video";
                break;
            default:
                if (e->hasFileContent()) {
                    type = "file";
                    break;
                }
                type = "message";
            }

        } else if (is<const StickerEvent>(replyEvt)) {
            type = "sticker";
        } else {
            type = "other";
        }

        QVariant content;
        if (auto e = eventCast<const RoomMessageEvent>(&replyEvt)) {
            // Cannot use e.contentJson() here because some
            // EventContent classes inject values into the copy of the
            // content JSON stored in EventContent::Base
            content = e->hasFileContent() ? QVariant::fromValue(e->content()->originalJson) : QVariant();
        };

        if (auto e = eventCast<const StickerEvent>(&replyEvt)) {
            content = QVariant::fromValue(e->image().originalJson);
        }

        return QVariantMap{{"eventId", replyEventId},
                           {"display", m_currentRoom->eventToString(replyEvt, Qt::RichText)},
                           {"content", content},
                           {"type", type},
                           {"author", userAtEvent(static_cast<NeoChatUser *>(m_currentRoom->user(replyEvt.senderId())), m_currentRoom, evt)}};
    }

    if (role == ShowAuthorRole) {
        for (auto r = row + 1; r < rowCount(); ++r) {
            auto i = index(r);
            if (data(i, SpecialMarksRole) != EventStatus::Hidden) {
                return data(i, AuthorRole) != data(idx, AuthorRole) || data(i, EventTypeRole) != data(idx, EventTypeRole)
                    || data(idx, TimeRole).toDateTime().msecsTo(data(i, TimeRole).toDateTime()) > 600000
                    || data(idx, TimeRole).toDateTime().toLocalTime().date().day() != data(i, TimeRole).toDateTime().toLocalTime().date().day();
            }
        }

        return true;
    }

    if (role == ShowSectionRole) {
        for (auto r = row + 1; r < rowCount(); ++r) {
            auto i = index(r);
            if (data(i, SpecialMarksRole) != EventStatus::Hidden) {
                const auto day = data(idx, TimeRole).toDateTime().toLocalTime().date().dayOfYear();
                const auto previousEventDay = data(i, TimeRole).toDateTime().toLocalTime().date().dayOfYear();
                return day != previousEventDay;
            }
        }

        return false;
    }

    if (role == ReactionRole) {
        const auto &annotations = m_currentRoom->relatedEvents(evt, EventRelation::Annotation());
        if (annotations.isEmpty()) {
            return {};
        };
        QMap<QString, QList<NeoChatUser *>> reactions = {};
        for (const auto &a : annotations) {
            if (a->isRedacted()) { // Just in case?
                continue;
            }
            if (auto e = eventCast<const ReactionEvent>(a)) {
                reactions[e->relation().key].append(static_cast<NeoChatUser *>(m_currentRoom->user(e->senderId())));
            }
        }

        if (reactions.isEmpty()) {
            return {};
        }

        QVariantList res = {};
        auto i = reactions.constBegin();
        while (i != reactions.constEnd()) {
            QVariantList authors;
            for (auto author : i.value()) {
                authors.append(userAtEvent(author, m_currentRoom, evt));
            }
            bool hasLocalUser = i.value().contains(static_cast<NeoChatUser *>(m_currentRoom->localUser()));
            res.append(QVariantMap{{"reaction", i.key()}, {"count", i.value().count()}, {"authors", authors}, {"hasLocalUser", hasLocalUser}});
            ++i;
        }

        return res;
    }

    return {};
}

int MessageEventModel::eventIDToIndex(const QString &eventID) const
{
    const auto it = m_currentRoom->findInTimeline(eventID);
    if (it == m_currentRoom->historyEdge()) {
        // qWarning() << "Trying to find inexistent event:" << eventID;
        return -1;
    }
    return it - m_currentRoom->messageEvents().rbegin() + timelineBaseIndex();
}

QVariant MessageEventModel::getLastLocalUserMessageEventId()
{
    QVariantMap targetMessage;
    const auto &timelineBottom = m_currentRoom->messageEvents().rbegin();

    // set a cap limit of 35 messages, to prevent loading a lot of messages
    // in rooms where the user has not sent many messages
    const auto limit = timelineBottom + std::min(35, m_currentRoom->timelineSize());

    for (auto it = timelineBottom; it != limit; ++it) {
        auto evt = it->event();
        auto e = eventCast<const RoomMessageEvent>(evt);

        // check if the current message's sender's id is same as the user's id
        if ((*it)->senderId() == m_currentRoom->localUser()->id()) {
            auto content = (*it)->contentJson();

            if (content.contains("m.relates_to")) {
                // the message has been edited once
                // so we have to return the id of the related' message instead
                targetMessage.insert("event_id", content["m.relates_to"].toObject()["event_id"].toString());
                targetMessage.insert("body", content["formatted_body"].toString());
                return targetMessage;
            }

            if (e->msgtype() == MessageEventType::Text) {
                targetMessage.insert("event_id", (*it)->id());
                targetMessage.insert("body", content["body"].toString());
                return targetMessage;
            }
        }
    }
    return targetMessage;
}

QVariant MessageEventModel::getLatestMessageFromIndex(const int baseline)
{
    QVariantMap replyResponse;
    const auto &timelineBottom = m_currentRoom->messageEvents().rbegin() + baseline;

    // set a cap limit of baseline + 35 messages, to prevent loading a lot of messages
    // in rooms where the user has not sent many messages
    const auto limit = timelineBottom + std::min(baseline + 35, m_currentRoom->timelineSize());

    for (auto it = timelineBottom; it != limit; ++it) {
        auto evt = it->event();
        auto e = eventCast<const RoomMessageEvent>(evt);

        auto content = (*it)->contentJson();

        if (content.contains("m.relates_to")) {
            auto relatedContent = content["m.relates_to"].toObject();

            if (!relatedContent.contains("m.in_reply_to")) {
                // the message has been edited once
                // so we have to return the id of the related' message instead
                replyResponse.insert("event_id", relatedContent["event_id"].toString());
                replyResponse.insert("event", content["m.formatted_body"].toString());
                replyResponse.insert("sender_id", QVariant::fromValue(m_currentRoom->getUser((*it)->senderId())));
                replyResponse.insert("at", -it->index());
                return replyResponse;
            }
        }

        if (e->msgtype() != MessageEventType::Unknown) {
            replyResponse.insert("event_id", (*it)->id());
            replyResponse.insert("event", content["body"].toString());
            replyResponse.insert("sender_id", QVariant::fromValue(m_currentRoom->getUser((*it)->senderId())));
            replyResponse.insert("at", -it->index());
            return replyResponse;
        }
    }
    return replyResponse;
}
