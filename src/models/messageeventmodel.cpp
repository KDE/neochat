// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "messageeventmodel.h"
#include "messageeventmodel_logging.h"

#include "neochatconfig.h"
#include <connection.h>
#include <csapi/rooms.h>
#include <events/reactionevent.h>
#include <events/redactionevent.h>
#include <events/roomavatarevent.h>
#include <events/roommemberevent.h>
#include <events/simplestateevents.h>
#include <user.h>

#ifdef QUOTIENT_07
#include "events/pollevent.h"
#endif
#include "events/stickerevent.h"

#include <QDebug>
#include <QGuiApplication>
#include <QQmlEngine> // for qmlRegisterType()
#include <QTimeZone>

#include <KFormat>
#include <KLocalizedString>

#include "neochatuser.h"
#include "texthandler.h"

using namespace Quotient;

QHash<int, QByteArray> MessageEventModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[DelegateTypeRole] = "delegateType";
    roles[PlainText] = "plainText";
    roles[EventIdRole] = "eventId";
    roles[TimeRole] = "time";
    roles[SectionRole] = "section";
    roles[AuthorRole] = "author";
    roles[ContentRole] = "content";
    roles[HighlightRole] = "isHighlighted";
    roles[SpecialMarksRole] = "marks";
    roles[LongOperationRole] = "progressInfo";
    roles[EventResolvedTypeRole] = "eventResolvedType";
    roles[ShowLinkPreviewRole] = "showLinkPreview";
    roles[LinkPreviewRole] = "linkPreview";
    roles[MediaInfoRole] = "mediaInfo";
    roles[IsReplyRole] = "isReply";
    roles[ReplyAuthor] = "replyAuthor";
    roles[ReplyRole] = "reply";
    roles[ReplyIdRole] = "replyId";
    roles[ReplyMediaInfoRole] = "replyMediaInfo";
    roles[ShowAuthorRole] = "showAuthor";
    roles[ShowSectionRole] = "showSection";
    roles[ReadMarkersRole] = "readMarkers";
    roles[ReadMarkersStringRole] = "readMarkersString";
    roles[ShowReadMarkersRole] = "showReadMarkers";
    roles[ReactionRole] = "reaction";
    roles[SourceRole] = "source";
    roles[MimeTypeRole] = "mimeType";
    roles[FormattedBodyRole] = "formattedBody";
    roles[AuthorIdRole] = "authorId";
    roles[VerifiedRole] = "verified";
    roles[DisplayNameForInitialsRole] = "displayNameForInitials";
    roles[AuthorDisplayNameRole] = "authorDisplayName";
    roles[IsRedactedRole] = "isRedacted";
    roles[GenericDisplayRole] = "genericDisplay";
    roles[IsPendingRole] = "isPending";
    roles[LatitudeRole] = "latitude";
    roles[LongitudeRole] = "longitude";
    roles[AssetRole] = "asset";
    return roles;
}

MessageEventModel::MessageEventModel(QObject *parent)
    : QAbstractListModel(parent)
{
    using namespace Quotient;
    qmlRegisterAnonymousType<FileTransferInfo>("org.kde.neochat", 1);
    qRegisterMetaType<FileTransferInfo>();

    connect(static_cast<QGuiApplication *>(QGuiApplication::instance()), &QGuiApplication::paletteChanged, this, [this] {
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole, ReplyRole});
    });
}

MessageEventModel::~MessageEventModel() = default;

NeoChatRoom *MessageEventModel::room() const
{
    return m_currentRoom;
}

void MessageEventModel::setRoom(NeoChatRoom *room)
{
    if (room == m_currentRoom) {
        return;
    }

    beginResetModel();
    if (m_currentRoom) {
        m_currentRoom->disconnect(this);
        m_linkPreviewers.clear();
    }

    m_currentRoom = room;
    if (room) {
        m_lastReadEventIndex = QPersistentModelIndex(QModelIndex());
        room->setDisplayed();

        for (auto event = m_currentRoom->messageEvents().begin(); event != m_currentRoom->messageEvents().end(); ++event) {
            if (auto e = &*event->viewAs<RoomMessageEvent>()) {
                createLinkPreviewerForEvent(e);
            }
        }

        if (m_currentRoom->timelineSize() < 10 && !room->allHistoryLoaded()) {
            room->getPreviousContent(50);
        }
#ifdef QUOTIENT_07
        lastReadEventId = room->lastFullyReadEventId();
#else
        lastReadEventId = room->readMarkerEventId();
#endif

        using namespace Quotient;
        connect(m_currentRoom, &Room::aboutToAddNewMessages, this, [this](RoomEventsRange events) {
            for (auto &&event : events) {
                const RoomMessageEvent *message = dynamic_cast<RoomMessageEvent *>(event.get());
                if (message != nullptr) {
                    createLinkPreviewerForEvent(message);

                    if (NeoChatConfig::self()->showFancyEffects()) {
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
            m_initialized = true;
            beginInsertRows({}, timelineBaseIndex(), timelineBaseIndex() + int(events.size()) - 1);
        });
        connect(m_currentRoom, &Room::aboutToAddHistoricalMessages, this, [this](RoomEventsRange events) {
            for (auto &event : events) {
                RoomMessageEvent *message = dynamic_cast<RoomMessageEvent *>(event.get());
                if (message) {
                    createLinkPreviewerForEvent(message);
                }
            }
            if (rowCount() > 0) {
                rowBelowInserted = rowCount() - 1; // See #312
            }
            m_initialized = true;
            beginInsertRows({}, rowCount(), rowCount() + int(events.size()) - 1);
        });
        connect(m_currentRoom, &Room::addedMessages, this, [this](int lowest, int biggest) {
            if (m_initialized) {
                endInsertRows();
            }
            if (!m_lastReadEventIndex.isValid()) {
                // no read marker, so see if we need to create one.
#ifdef QUOTIENT_07
                moveReadMarker(m_currentRoom->lastFullyReadEventId());
#else
                moveReadMarker(m_currentRoom->readMarkerEventId());
#endif
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
            m_initialized = true;
            beginInsertRows({}, 0, 0);
        });
        connect(m_currentRoom, &Room::pendingEventAdded, this, &MessageEventModel::endInsertRows);
        connect(m_currentRoom, &Room::pendingEventAboutToMerge, this, [this](RoomEvent *, int i) {
            Q_EMIT dataChanged(index(i, 0), index(i, 0), {IsPendingRole});
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
        connect(m_currentRoom, &Room::readMarkerMoved, this, [this](const QString &fromEventId, const QString &toEventId) {
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
        connect(m_currentRoom, &Room::changed, this, [this]() {
            for (auto it = m_currentRoom->messageEvents().rbegin(); it != m_currentRoom->messageEvents().rend(); ++it) {
                auto event = it->event();
                refreshEventRoles(event->id(), {ReadMarkersRole, ReadMarkersStringRole});
            }
        });
        connect(m_currentRoom, &Room::newFileTransfer, this, &MessageEventModel::refreshEvent);
        connect(m_currentRoom, &Room::fileTransferProgress, this, &MessageEventModel::refreshEvent);
        connect(m_currentRoom, &Room::fileTransferCompleted, this, &MessageEventModel::refreshEvent);
        connect(m_currentRoom, &Room::fileTransferFailed, this, &MessageEventModel::refreshEvent);
#ifndef QUOTIENT_07
        connect(m_currentRoom, &Room::fileTransferCancelled, this, &MessageEventModel::refreshEvent);
#endif
        connect(m_currentRoom->connection(), &Connection::ignoredUsersListChanged, this, [this] {
            beginResetModel();
            endResetModel();
        });
        qCDebug(MessageEvent) << "Connected to room" << room->id() << "as" << room->localUser()->id();
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
            m_initialized = true;
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
            return -1;
        }
        row = int(timelineIt - m_currentRoom->messageEvents().rbegin()) + timelineBaseIndex();
        if (data(index(row, 0), DelegateTypeRole).toInt() == ReadMarker || data(index(row, 0), DelegateTypeRole).toInt() == Other) {
            row++;
        }
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
    qCCritical(MessageEvent) << "No valid timestamps in the room timeline!";
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

static const QVariantMap emptyUser = {
    {"isLocalUser", false},
    {"id", QString()},
    {"avatarSource", QUrl()},
    {"avatarMediaId", QString()},
    {"avatarUrl", QString()},
    {"displayName", QString()},
    {"display", QString()},
    {"color", QColor()},
    {"object", QVariant()},
};

inline QVariantMap userInContext(NeoChatUser *user, NeoChatRoom *room)
{
    return QVariantMap{
        {"isLocalUser", user->id() == room->localUser()->id()},
        {"id", user->id()},
        {"avatarSource", room->avatarForMember(user)},
        {"avatarMediaId", user->avatarMediaId(room)},
        {"avatarUrl", user->avatarUrl(room)},
        {"displayName", user->displayname(room)},
        {"display", user->name()},
        {"color", user->color()},
        {"object", QVariant::fromValue(user)},
    };
}

static LinkPreviewer *emptyLinkPreview = new LinkPreviewer;

QVariant MessageEventModel::data(const QModelIndex &idx, int role) const
{
    const auto row = idx.row();

    if (!m_currentRoom || row < 0 || row >= int(m_currentRoom->pendingEvents().size()) + m_currentRoom->timelineSize()) {
        return {};
    };

    bool isPending = row < timelineBaseIndex();

    if (m_lastReadEventIndex.row() == row) {
        switch (role) {
        case DelegateTypeRole:
            return DelegateType::ReadMarker;
        case TimeRole: {
            const QDateTime eventDate = data(index(m_lastReadEventIndex.row() + 1, 0), TimeRole).toDateTime().toLocalTime();
            const KFormat format;
            return format.formatRelativeDateTime(eventDate, QLocale::ShortFormat);
        }
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

    if (role == GenericDisplayRole) {
        if (evt.isRedacted()) {
            return i18n("<i>[This message was deleted]</i>");
        }

        return m_currentRoom->eventToGenericString(evt);
    }

    if (role == FormattedBodyRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            if (e->hasTextContent() && e->mimeType().name() != "text/plain") {
                return static_cast<const Quotient::EventContent::TextContent *>(e->content())->body;
            }
        }

        return {};
    }

    if (role == PlainText) {
        return m_currentRoom->eventToString(evt);
    }

    if (role == SourceRole) {
        return QJsonDocument(evt.fullJson()).toJson();
    }

    if (role == DelegateTypeRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            switch (e->msgtype()) {
            case MessageEventType::Emote:
                return DelegateType::Emote;
            case MessageEventType::Notice:
                return DelegateType::Notice;
            case MessageEventType::Image:
                return DelegateType::Image;
            case MessageEventType::Audio:
                return DelegateType::Audio;
            case MessageEventType::Video:
                return DelegateType::Video;
            case MessageEventType::Location:
                return DelegateType::Location;
            default:
                break;
            }
            if (e->hasFileContent()) {
                return DelegateType::File;
            }

            return DelegateType::Message;
        }
        if (is<const StickerEvent>(evt)) {
            return DelegateType::Sticker;
        }
        if (evt.isStateEvent()) {
            return DelegateType::State;
        }
        if (is<const EncryptedEvent>(evt)) {
            return DelegateType::Encrypted;
        }
#ifdef QUOTIENT_07
        if (is<PollStartEvent>(evt)) {
            if (evt.isRedacted()) {
                return DelegateType::Message;
            }
            return DelegateType::Poll;
        }
#endif

        return DelegateType::Other;
    }

    if (role == EventResolvedTypeRole) {
        return EventTypeRegistry::getMatrixType(evt.type());
    }

    if (role == AuthorRole) {
        auto author = static_cast<NeoChatUser *>(isPending ? m_currentRoom->localUser() : m_currentRoom->user(evt.senderId()));
        return userInContext(author, m_currentRoom);
    }

    if (role == ContentRole) {
        if (evt.isRedacted()) {
            auto reason = evt.redactedBecause()->reason();
            return (reason.isEmpty()) ? i18n("[REDACTED]") : i18n("[REDACTED: %1]").arg(evt.redactedBecause()->reason());
        }

        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            if (e->msgtype() == Quotient::MessageEventType::Location) {
                return e->contentJson();
            }
            // Cannot use e.contentJson() here because some
            // EventContent classes inject values into the copy of the
            // content JSON stored in EventContent::Base
            return e->hasFileContent() ? QVariant::fromValue(e->content()->originalJson) : QVariant();
        };

        if (auto e = eventCast<const StickerEvent>(&evt)) {
            return QVariant::fromValue(e->image().originalJson);
        }
        return evt.contentJson();
    }

    if (role == HighlightRole) {
        return !m_currentRoom->isDirectChat() && m_currentRoom->isEventHighlighted(&evt);
    }

    if (role == MimeTypeRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            if (!e || !e->hasFileContent()) {
                return QVariant();
            }

            return e->content()->fileInfo()->mimeType.name();
        }

        if (auto e = eventCast<const StickerEvent>(&evt)) {
            return e->image().mimeType.name();
        }
    }

    if (role == SpecialMarksRole) {
        if (isPending) {
            // A pending event with an m.new_content key will be merged into the
            // original event so don't show.
            if (evt.contentJson().contains("m.new_content")) {
                return EventStatus::Hidden;
            }
            return pendingIt->deliveryStatus();
        }

        if (evt.isStateEvent() && !NeoChatConfig::self()->showStateEvent()) {
            return EventStatus::Hidden;
        }

        if (auto roomMemberEvent = eventCast<const RoomMemberEvent>(&evt)) {
            if ((roomMemberEvent->isJoin() || roomMemberEvent->isLeave()) && !NeoChatConfig::self()->showLeaveJoinEvent()) {
                return EventStatus::Hidden;
            } else if (roomMemberEvent->isRename() && !roomMemberEvent->isJoin() && !roomMemberEvent->isLeave() && !NeoChatConfig::self()->showRename()) {
                return EventStatus::Hidden;
            } else if (roomMemberEvent->isAvatarUpdate() && !roomMemberEvent->isJoin() && !roomMemberEvent->isLeave()
                       && !NeoChatConfig::self()->showAvatarUpdate()) {
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

    if (role == TimeRole || role == SectionRole) {
        auto ts = isPending ? pendingIt->lastUpdated() : makeMessageTimestamp(timelineIt);
        return role == TimeRole ? QVariant(ts) : renderDate(ts);
    }

    if (role == ShowLinkPreviewRole) {
        return m_linkPreviewers.contains(evt.id());
    }

    if (role == LinkPreviewRole) {
        if (m_linkPreviewers.contains(evt.id())) {
            return QVariant::fromValue<LinkPreviewer *>(m_linkPreviewers[evt.id()]);
        } else {
            return QVariant::fromValue<LinkPreviewer *>(emptyLinkPreview);
        }
    }

    if (role == MediaInfoRole) {
        return getMediaInfoForEvent(evt);
    }

    if (role == IsReplyRole) {
        return !evt.contentJson()["m.relates_to"].toObject()["m.in_reply_to"].toObject()["event_id"].toString().isEmpty();
    }

    if (role == ReplyIdRole) {
        return evt.contentJson()["m.relates_to"].toObject()["m.in_reply_to"].toObject()["event_id"].toString();
    }

    if (role == ReplyAuthor) {
        auto replyPtr = getReplyForEvent(evt);

        if (replyPtr) {
            auto replyUser = static_cast<NeoChatUser *>(m_currentRoom->user(replyPtr->senderId()));
            return userInContext(replyUser, m_currentRoom);
        } else {
            return emptyUser;
        }
    }

    if (role == ReplyMediaInfoRole) {
        auto replyPtr = getReplyForEvent(evt);
        if (!replyPtr) {
            return {};
        }
        return getMediaInfoForEvent(*replyPtr);
    }

    if (role == ReplyRole) {
        auto replyPtr = getReplyForEvent(evt);
        if (!replyPtr) {
            return {};
        }

        DelegateType type;
        if (auto e = eventCast<const RoomMessageEvent>(replyPtr)) {
            switch (e->msgtype()) {
            case MessageEventType::Emote:
                type = DelegateType::Emote;
                break;
            case MessageEventType::Notice:
                type = DelegateType::Notice;
                break;
            case MessageEventType::Image:
                type = DelegateType::Image;
                break;
            case MessageEventType::Audio:
                type = DelegateType::Audio;
                break;
            case MessageEventType::Video:
                type = DelegateType::Video;
                break;
            default:
                if (e->hasFileContent()) {
                    type = DelegateType::File;
                    break;
                }
                type = DelegateType::Message;
            }

        } else if (is<const StickerEvent>(*replyPtr)) {
            type = DelegateType::Sticker;
        } else {
            type = DelegateType::Other;
        }

        return QVariantMap{
            {"display", m_currentRoom->eventToString(*replyPtr, Qt::RichText)},
            {"type", type},
        };
    }

    if (role == ShowAuthorRole) {
        for (auto r = row + 1; r < rowCount(); ++r) {
            auto i = index(r);
            // Note !itemData(i).empty() is a check for instances where rows have been removed, e.g. when the read marker is moved.
            // While the row is removed the subsequent row indexes are not changed so we need to skip over the removed index.
            // See - https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
            if (data(i, SpecialMarksRole) != EventStatus::Hidden && !itemData(i).empty()) {
                return data(i, AuthorRole) != data(idx, AuthorRole) || data(i, DelegateTypeRole) == MessageEventModel::State
                    || data(i, TimeRole).toDateTime().msecsTo(data(idx, TimeRole).toDateTime()) > 600000
                    || data(i, TimeRole).toDateTime().toLocalTime().date().day() != data(idx, TimeRole).toDateTime().toLocalTime().date().day();
            }
        }

        return true;
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

    if (role == LatitudeRole) {
        const auto geoUri = evt.contentJson()["geo_uri"_ls].toString();
        if (geoUri.isEmpty()) {
            return {};
        }
        const auto latitude = geoUri.split(u';')[0].split(u':')[1].split(u',')[0];
        return latitude.toFloat();
    }

    if (role == LongitudeRole) {
        const auto geoUri = evt.contentJson()["geo_uri"_ls].toString();
        if (geoUri.isEmpty()) {
            return {};
        }
        const auto latitude = geoUri.split(u';')[0].split(u':')[1].split(u',')[1];
        return latitude.toFloat();
    }

    if (role == AssetRole) {
        const auto assetType = evt.contentJson()["org.matrix.msc3488.asset"].toObject()["type"].toString();
        if (assetType.isEmpty()) {
            return {};
        }
        return assetType;
    }

    if (role == ReadMarkersRole) {
#ifdef QUOTIENT_07
        auto userIds = room()->userIdsAtEvent(evt.id());
        userIds.remove(m_currentRoom->localUser()->id());
#else
        auto userIds = room()->usersAtEventId(evt.id());
        userIds.removeAll(m_currentRoom->localUser());
#endif

        QVariantList users;
        users.reserve(userIds.size());
        for (const auto &userId : userIds) {
#ifdef QUOTIENT_07
            auto user = static_cast<NeoChatUser *>(m_currentRoom->user(userId));
#else
            auto user = static_cast<NeoChatUser *>(userId);
#endif
            users += userInContext(user, m_currentRoom);
        }

        return users;
    }

    if (role == ReadMarkersStringRole) {
#ifdef QUOTIENT_07
        auto userIds = room()->userIdsAtEvent(evt.id());
        userIds.remove(m_currentRoom->localUser()->id());
#else
        auto userIds = room()->usersAtEventId(evt.id());
        userIds.removeAll(m_currentRoom->localUser());
#endif
        /**
         * The string ends up in the form
         * "x users: user1DisplayName, user2DisplayName, etc."
         */
        QString readMarkersString = i18np("1 user: ", "%1 users: ", userIds.size());
        for (const auto &userId : userIds) {
#ifdef QUOTIENT_07
            auto user = static_cast<NeoChatUser *>(m_currentRoom->user(userId));
#else
            auto user = static_cast<NeoChatUser *>(userId);
#endif
            readMarkersString += user->displayname(m_currentRoom) + i18nc("list separator", ", ");
        }
        readMarkersString.chop(2);
        return readMarkersString;
    }

    if (role == ShowReadMarkersRole) {
#ifdef QUOTIENT_07
        auto userIds = room()->userIdsAtEvent(evt.id());
        userIds.remove(m_currentRoom->localUser()->id());
#else
        auto userIds = room()->usersAtEventId(evt.id());
        userIds.removeAll(m_currentRoom->localUser());
#endif
        return userIds.size() > 0;
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
                authors.append(userInContext(author, m_currentRoom));
            }
            bool hasLocalUser = i.value().contains(static_cast<NeoChatUser *>(m_currentRoom->localUser()));
            res.append(QVariantMap{{"reaction", i.key()}, {"count", i.value().count()}, {"authors", authors}, {"hasLocalUser", hasLocalUser}});
            ++i;
        }

        return res;
    }
    if (role == AuthorIdRole) {
        return evt.senderId();
    }

    if (role == VerifiedRole) {
#ifdef QUOTIENT_07
#ifdef Quotient_E2EE_ENABLED
        if (evt.originalEvent()) {
            auto encrypted = dynamic_cast<const EncryptedEvent *>(evt.originalEvent());
            Q_ASSERT(encrypted);
            return m_currentRoom->connection()->isVerifiedSession(encrypted->sessionId().toLatin1());
        }
#endif
#endif
        return false;
    }

    if (role == DisplayNameForInitialsRole) {
        auto user = static_cast<NeoChatUser *>(isPending ? m_currentRoom->localUser() : m_currentRoom->user(evt.senderId()));
        return user->displayname(m_currentRoom).remove(QStringLiteral(" (%1)").arg(user->id()));
    }

    if (role == AuthorDisplayNameRole) {
        if (is<RoomMemberEvent>(evt) && !evt.unsignedJson()["prev_content"]["displayname"].isNull() && evt.stateKey() == evt.senderId()) {
            auto previousDisplayName = evt.unsignedJson()["prev_content"]["displayname"].toString().toHtmlEscaped();
            if (previousDisplayName.isEmpty()) {
                previousDisplayName = evt.senderId();
            }
            return previousDisplayName;
        } else {
            auto author = static_cast<NeoChatUser *>(isPending ? m_currentRoom->localUser() : m_currentRoom->user(evt.senderId()));
            return m_currentRoom->htmlSafeMemberName(author->id());
        }
    }

    if (role == IsRedactedRole) {
        return evt.isRedacted();
    }

    if (role == IsPendingRole) {
        return row < static_cast<int>(m_currentRoom->pendingEvents().size());
    }

    return {};
}

int MessageEventModel::eventIdToRow(const QString &eventID) const
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
        if (!e) {
            return {};
        }

        // check if the current message's sender's id is same as the user's id
        if ((*it)->senderId() == m_currentRoom->localUser()->id()) {
            auto content = (*it)->contentJson();

            if (e->msgtype() != MessageEventType::Unknown) {
                QString eventId;
                if (content.contains("m.new_content")) {
                    // The message has been edited so we have to return the id of the original message instead of the replacement
                    eventId = content["m.relates_to"].toObject()["event_id"].toString();
                } else {
                    // For any message that isn't an edit return the id of the current message
                    eventId = (*it)->id();
                }
                targetMessage.insert("event_id", eventId);
                targetMessage.insert("formattedBody", content["formatted_body"].toString());
                // Need to get the message from the original eventId or body will have * on the front
                QModelIndex idx = index(eventIdToRow(eventId), 0);
                targetMessage.insert("message", idx.data(Qt::UserRole + 2));

                return targetMessage;
            }
        }
    }
    return targetMessage;
}

QVariant MessageEventModel::getLatestMessageFromRow(const int startRow)
{
    QVariantMap replyResponse;
    const auto &timelineBottom = m_currentRoom->messageEvents().rbegin() + startRow;

    // set a cap limit of startRow + 35 messages, to prevent loading a lot of messages
    // in rooms where the user has not sent many messages
    const auto limit = timelineBottom + std::min(startRow + 35, m_currentRoom->timelineSize());

    for (auto it = timelineBottom; it != limit; ++it) {
        auto evt = it->event();
        auto e = eventCast<const RoomMessageEvent>(evt);
        if (!e) {
            continue;
        }

        auto content = (*it)->contentJson();

        if (e->msgtype() != MessageEventType::Unknown) {
            QString eventId;
            if (content.contains("m.new_content")) {
                // The message has been edited so we have to return the id of the original message instead of the replacement
                eventId = content["m.relates_to"].toObject()["event_id"].toString();
            } else {
                // For any message that isn't an edit return the id of the current message
                eventId = (*it)->id();
            }
            replyResponse.insert("event_id", eventId);
            // Need to get the message from the original eventId or body will have * on the front
            QModelIndex idx = index(eventIdToRow(eventId), 0);
            replyResponse.insert("message", idx.data(Qt::UserRole + 2));
            replyResponse.insert("sender_id", QVariant::fromValue(m_currentRoom->getUser((*it)->senderId())));
            replyResponse.insert("at", -it->index());
            return replyResponse;
        }
    }
    return replyResponse;
}

void MessageEventModel::loadReply(const QModelIndex &index)
{
    auto job = m_currentRoom->connection()->callApi<GetOneRoomEventJob>(m_currentRoom->id(), data(index, ReplyIdRole).toString());
    QPersistentModelIndex persistentIndex(index);
    connect(job, &BaseJob::success, this, [this, job, persistentIndex] {
        m_extraEvents.push_back(fromJson<event_ptr_tt<RoomEvent>>(job->jsonData()));
        Q_EMIT dataChanged(persistentIndex, persistentIndex, {ReplyRole, ReplyMediaInfoRole, ReplyAuthor});
    });
}

const RoomEvent *MessageEventModel::getReplyForEvent(const RoomEvent &event) const
{
    const QString &replyEventId = event.contentJson()["m.relates_to"].toObject()["m.in_reply_to"].toObject()["event_id"].toString();
    if (replyEventId.isEmpty()) {
        return {};
    };

    const auto replyIt = m_currentRoom->findInTimeline(replyEventId);
    const RoomEvent *replyPtr = replyIt != m_currentRoom->historyEdge() ? &**replyIt : nullptr;
    if (!replyPtr) {
        for (const auto &e : m_extraEvents) {
            if (e->id() == replyEventId) {
                replyPtr = e.get();
                break;
            }
        }
    }
    return replyPtr;
}

QVariantMap MessageEventModel::getMediaInfoForEvent(const RoomEvent &event) const
{
    QVariantMap mediaInfo;

    QString eventId = event.id();

    // Get the file info for the event.
    const EventContent::FileInfo *fileInfo;
#ifdef QUOTIENT_07
    if (event.is<RoomMessageEvent>()) {
        auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event);
#else
    if (auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event)) {
#endif
        if (!roomMessageEvent->hasFileContent()) {
            return {};
        }
        fileInfo = roomMessageEvent->content()->fileInfo();
#ifdef QUOTIENT_07
    } else if (event.is<StickerEvent>()) {
        auto stickerEvent = eventCast<const StickerEvent>(&event);
#else
    } else if (auto stickerEvent = eventCast<const StickerEvent>(&event)) {
#endif
        fileInfo = &stickerEvent->image();
    } else {
        return {};
    }

    return getMediaInfoFromFileInfo(fileInfo, eventId);
}

QVariantMap MessageEventModel::getMediaInfoFromFileInfo(const EventContent::FileInfo *fileInfo, const QString &eventId, bool isThumbnail) const
{
    QVariantMap mediaInfo;

    // Get the mxc URL for the media.
#ifdef QUOTIENT_07
    if (!fileInfo->url().isValid() || eventId.isEmpty()) {
        mediaInfo["source"] = QUrl();
    } else {
        QUrl source = m_currentRoom->makeMediaUrl(eventId, fileInfo->url());

        if (source.isValid() && source.scheme() == QStringLiteral("mxc")) {
            mediaInfo["source"] = source;
        } else {
            mediaInfo["source"] = QUrl();
        }
    }
#else
    auto url = QUrl(m_currentRoom->connection()->homeserver().toString() + "/_matrix/media/r0/download/" + fileInfo->url.toString().remove("mxc://"));
    QUrlQuery q(url.query());
    q.addQueryItem("allow_remote", "true");
    url.setQuery(q);
    mediaInfo["source"] = url;
#endif

    auto mimeType = fileInfo->mimeType;
    // Add the MIME type for the media if available.
    mediaInfo["mimeType"] = mimeType.name();

    // Add the MIME type icon if available.
    mediaInfo["mimeIcon"] = mimeType.iconName();

    // Add media size if available.
    mediaInfo["size"] = fileInfo->payloadSize;

    // Add parameter depending on media type.
    if (mimeType.name().contains(QStringLiteral("image"))) {
        if (auto castInfo = static_cast<const EventContent::ImageContent *>(fileInfo)) {
            mediaInfo["width"] = castInfo->imageSize.width();
            mediaInfo["height"] = castInfo->imageSize.height();

            if (!isThumbnail) {
                QVariantMap tempInfo;
                auto thumbnailInfo = getMediaInfoFromFileInfo(castInfo->thumbnailInfo(), eventId, true);
                if (thumbnailInfo["source"].toUrl().scheme() == "mxc") {
                    tempInfo = thumbnailInfo;
                } else {
                    QString blurhash = castInfo->originalInfoJson["xyz.amorgan.blurhash"].toString();
                    if (blurhash.isEmpty()) {
                        tempInfo["source"] = QUrl();
                    } else {
                        tempInfo["source"] = QUrl("image://blurhash/" + blurhash);
                    }
                }
                mediaInfo["tempInfo"] = tempInfo;
            }
        }
    }
    if (mimeType.name().contains(QStringLiteral("video"))) {
        if (auto castInfo = static_cast<const EventContent::VideoContent *>(fileInfo)) {
            mediaInfo["width"] = castInfo->imageSize.width();
            mediaInfo["height"] = castInfo->imageSize.height();
            mediaInfo["duration"] = castInfo->duration;

            if (!isThumbnail) {
                QVariantMap tempInfo;
                auto thumbnailInfo = getMediaInfoFromFileInfo(castInfo->thumbnailInfo(), eventId, true);
                if (thumbnailInfo["source"].toUrl().scheme() == "mxc") {
                    tempInfo = thumbnailInfo;
                } else {
                    QString blurhash = castInfo->originalInfoJson["xyz.amorgan.blurhash"].toString();
                    if (blurhash.isEmpty()) {
                        tempInfo["source"] = QUrl();
                    } else {
                        tempInfo["source"] = QUrl("image://blurhash/" + blurhash);
                    }
                }
                mediaInfo["tempInfo"] = tempInfo;
            }
        }
    }
    if (mimeType.name().contains(QStringLiteral("audio"))) {
        if (auto castInfo = static_cast<const EventContent::AudioContent *>(fileInfo)) {
            mediaInfo["duration"] = castInfo->duration;
        }
    }

    return mediaInfo;
}

void MessageEventModel::createLinkPreviewerForEvent(const Quotient::RoomMessageEvent *event)
{
    if (m_linkPreviewers.contains(event->id())) {
        return;
    } else {
        QString text;
        if (event->hasTextContent()) {
            auto textContent = static_cast<const EventContent::TextContent *>(event->content());
            if (textContent) {
                text = textContent->body;
            } else {
                text = event->plainBody();
            }
        } else {
            text = event->plainBody();
        }
        TextHandler textHandler;
        textHandler.setData(text);

        QList<QUrl> links = textHandler.getLinkPreviews();
        if (links.size() > 0) {
            m_linkPreviewers[event->id()] = new LinkPreviewer(nullptr, m_currentRoom, links.size() > 0 ? links[0] : QUrl());
        }
    }
}
