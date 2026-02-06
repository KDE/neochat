// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "eventmessagecontentmodel.h"

#include <Quotient/events/eventcontent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/qt_connection_util.h>
#include <Quotient/thread.h>

#include <KLocalizedString>
#include <Kirigami/Platform/PlatformTheme>

#include "chatbarcache.h"
#include "contentprovider.h"
#include "eventhandler.h"
#include "models/reactionmodel.h"
#include "neochatdatetime.h"
#include "neochatroom.h"
#include "texthandler.h"

using namespace Quotient;

bool EventMessageContentModel::m_threadsEnabled = false;

EventMessageContentModel::EventMessageContentModel(NeoChatRoom *room, const QString &eventId, bool isReply, bool isPending, MessageContentModel *parent)
    : MessageContentModel(room, parent, eventId)
    , m_currentState(isPending ? Pending : Unknown)
    , m_isReply(isReply)
{
    initializeModel();
}

void EventMessageContentModel::initializeModel()
{
    Q_ASSERT(m_room != nullptr);
    Q_ASSERT(!m_eventId.isEmpty());

    connect(m_room, &NeoChatRoom::pendingEventAdded, this, [this]() {
        if (m_room != nullptr && m_currentState == Unknown) {
            initializeEvent();
            resetModel();
        }
    });
    connect(m_room, &NeoChatRoom::pendingEventAboutToMerge, this, [this](Quotient::RoomEvent *serverEvent) {
        if (m_room != nullptr) {
            if (m_eventId == serverEvent->id() || m_eventId == serverEvent->transactionId()) {
                m_eventId = serverEvent->id();
            }
        }
    });
    connect(m_room, &NeoChatRoom::pendingEventMerged, this, [this]() {
        if (m_room != nullptr && m_currentState == Pending) {
            initializeEvent();
            resetModel();
        }
    });
    connect(m_room, &NeoChatRoom::addedMessages, this, [this](int fromIndex, int toIndex) {
        if (!m_room) {
            return;
        }
        for (int i = fromIndex; i <= toIndex; i++) {
            if (m_room->findInTimeline(i)->event()->id() == m_eventId) {
                initializeEvent();
                resetModel();
            }
        }
    });
    connect(m_room, &NeoChatRoom::replacedEvent, this, [this](const Quotient::RoomEvent *newEvent) {
        if (m_room != nullptr) {
            if (m_eventId == newEvent->id()) {
                initializeEvent();
                resetContent();
            }
        }
    });
    connect(m_room->editCache(), &ChatBarCache::relationIdChanged, this, [this](const QString &oldEventId, const QString &newEventId) {
        if (oldEventId == m_eventId || newEventId == m_eventId) {
            resetContent(newEventId == m_eventId);
        }
    });
    connect(m_room->threadCache(), &ChatBarCache::threadIdChanged, this, [this](const QString &oldThreadId, const QString &newThreadId) {
        if (oldThreadId == m_eventId || newThreadId == m_eventId) {
            resetContent(false, newThreadId == m_eventId);
        }
    });
    connect(m_room, &NeoChatRoom::urlPreviewEnabledChanged, this, [this]() {
        resetContent();
    });
    connect(m_room, &Room::memberNameUpdated, this, [this](RoomMember member) {
        if (m_room != nullptr) {
            if (authorId().isEmpty() || authorId() == member.id()) {
                Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole});
                Q_EMIT authorChanged();
            }
        }
    });
    connect(m_room, &Room::memberAvatarUpdated, this, [this](RoomMember member) {
        if (m_room != nullptr) {
            if (authorId().isEmpty() || authorId() == member.id()) {
                Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole});
                Q_EMIT authorChanged();
            }
        }
    });
    connect(this, &EventMessageContentModel::threadsEnabledChanged, this, [this]() {
        resetModel();
    });
    connect(m_room, &Room::updatedEvent, this, [this](const QString &eventId) {
        if (eventId == m_eventId) {
            updateReactionModel();
        }
    });
#if Quotient_VERSION_MINOR > 9
    connect(m_room, &Room::newThread, this, [this](const auto &newThread) {
        if (newThread == m_eventId) {
            resetContent();
        }
    });
#endif

    initializeEvent();
    resetModel();
}

NeoChatDateTime EventMessageContentModel::dateTime() const
{
    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr) {
        return MessageContentModel::dateTime();
    };
    return EventHandler::dateTime(m_room, event.first, m_currentState == Pending);
}

QString EventMessageContentModel::authorId() const
{
    const auto eventResult = m_room->getEvent(m_eventId);
    if (eventResult.first == nullptr) {
        return {};
    }
    auto authorId = eventResult.first->senderId();
    if (authorId.isEmpty()) {
        return MessageContentModel::authorId();
    }
    return authorId;
}

QString EventMessageContentModel::threadRootId() const
{
    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr) {
        return {};
    }
    auto roomMessageEvent = eventCast<const RoomMessageEvent>(event.first);
    if (roomMessageEvent && roomMessageEvent->isThreaded()) {
        return roomMessageEvent->threadRootEventId();
    } else if (m_room->threads().contains(roomMessageEvent->id())) {
        return m_eventId;
    }
    return {};
}

void EventMessageContentModel::initializeEvent()
{
    if (m_currentState == UnAvailable) {
        return;
    }

    const auto eventResult = m_room->getEvent(m_eventId);
    if (eventResult.first == nullptr) {
        if (m_currentState != Pending) {
            getEvent();
        }
        return;
    }
    if (eventResult.second) {
        m_currentState = Pending;
    } else {
        m_currentState = Available;
    }
    Q_EMIT eventUpdated();
}

void EventMessageContentModel::getEvent()
{
    Quotient::connectUntil(m_room.get(), &NeoChatRoom::extraEventLoaded, this, [this](const QString &eventId) {
        if (m_room != nullptr) {
            if (eventId == m_eventId) {
                initializeEvent();
                resetModel();
                return true;
            }
        }
        return false;
    });
    Quotient::connectUntil(m_room.get(), &NeoChatRoom::extraEventNotFound, this, [this](const QString &eventId) {
        if (m_room != nullptr) {
            if (eventId == m_eventId) {
                m_currentState = UnAvailable;
                resetModel();
                return true;
            }
        }
        return false;
    });

    m_room->downloadEventFromServer(m_eventId);
}

MessageComponent EventMessageContentModel::unavailableMessageComponent() const
{
    const auto theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));

    QString disabledTextColor;
    if (theme != nullptr) {
        disabledTextColor = theme->disabledTextColor().name();
    } else {
        disabledTextColor = u"#000000"_s;
    }

    return MessageComponent{
        .type = MessageComponentType::Text,
        .display = u"<span style=\"color:%1\">"_s.arg(disabledTextColor)
            + i18nc("@info", "This message was either not found, you do not have permission to view it, or it was sent by an ignored user") + u"</span>"_s,
        .attributes = {},
    };
}

void EventMessageContentModel::resetModel()
{
    beginResetModel();
    m_components.clear();

    if (m_room->connection()->isIgnored(authorId()) || m_currentState == UnAvailable) {
        m_components += unavailableMessageComponent();
        endResetModel();
        return;
    }

    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr) {
        m_components +=
            MessageComponent{MessageComponentType::Loading, m_isReply ? i18nc("@info", "Loading reply…") : i18nc("@info Loading this message", "Loading…"), {}};
        endResetModel();
        return;
    }

    m_components += MessageComponent{MessageComponentType::Author, {}, {}};

    m_components += messageContentComponents();
    endResetModel();

    updateReplyModel();
    updateReactionModel();
    updateItineraryModel();

    Q_EMIT componentsUpdated();
    // We need QML to re-evaluate author (for example, reply colors) if it was previously null.
    Q_EMIT authorChanged();
}

void EventMessageContentModel::resetContent(bool isEditing, bool isThreading)
{
    const auto startRow = m_components[0].type == MessageComponentType::Author ? 1 : 0;
    beginRemoveRows({}, startRow, rowCount() - 1);
    m_components.remove(startRow, rowCount() - startRow);
    endRemoveRows();

    const auto newComponents = messageContentComponents(isEditing, isThreading);
    if (newComponents.size() == 0) {
        return;
    }
    beginInsertRows({}, startRow, startRow + newComponents.size() - 1);
    m_components += newComponents;
    endInsertRows();

    updateReplyModel();
    updateReactionModel();
    updateItineraryModel();

    Q_EMIT componentsUpdated();
}

QList<MessageComponent> EventMessageContentModel::messageContentComponents(bool isEditing, bool isThreading)
{
    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr) {
        return {};
    }

    QList<MessageComponent> newComponents;

    if (isEditing) {
        newComponents += MessageComponent{MessageComponentType::ChatBar, QString(), {}};
    } else {
        newComponents.append(componentsForType(MessageComponentType::typeForEvent(*event.first, m_isReply)));
    }

    const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event.first);
    if (m_threadsEnabled && roomMessageEvent
        && ((roomMessageEvent->isThreaded() && roomMessageEvent->id() == roomMessageEvent->threadRootEventId())
            || m_room->threads().contains(roomMessageEvent->id()))) {
        newComponents += MessageComponent{MessageComponentType::Separator, {}, {}};
        newComponents += MessageComponent{MessageComponentType::ThreadBody, u"Thread Body"_s, {}};
    }

    // If the event is already threaded the ThreadModel will handle displaying a chat bar.
    if (isThreading && roomMessageEvent && !(roomMessageEvent->isThreaded() || m_room->threads().contains(roomMessageEvent->id()))) {
        newComponents += MessageComponent{MessageComponentType::ChatBar, QString(), {}};
    }

    return newComponents;
}

void EventMessageContentModel::updateReplyModel()
{
    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr || m_isReply) {
        return;
    }

    const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event.first);
    if (roomMessageEvent == nullptr) {
        return;
    }
    if (!roomMessageEvent->isReply(m_threadsEnabled) || (roomMessageEvent->isThreaded() && m_threadsEnabled)) {
        if (m_replyModel) {
            m_replyModel->disconnect(this);
            m_replyModel->deleteLater();
        }
        return;
    }

    m_replyModel = new EventMessageContentModel(m_room, roomMessageEvent->replyEventId(!m_threadsEnabled), true, false, this);

    bool hasModel = hasComponentType(MessageComponentType::Reply);
    if (m_replyModel && !hasModel) {
        int insertRow = 0;
        if (m_components.first().type == MessageComponentType::Author) {
            insertRow = 1;
        }
        beginInsertRows({}, insertRow, insertRow);
        m_components.insert(insertRow, MessageComponent{MessageComponentType::Reply, QString(), {}});
    } else if (!m_replyModel && hasModel) {
        int removeRow = 0;
        if (m_components.first().type == MessageComponentType::Author) {
            removeRow = 1;
        }
        beginRemoveRows({}, removeRow, removeRow);
        m_components.removeAt(removeRow);
        endRemoveRows();
    }
}

QList<MessageComponent> EventMessageContentModel::componentsForType(MessageComponentType::Type type)
{
    const auto [event, _] = m_room->getEvent(m_eventId);
    if (event == nullptr) {
        return {};
    }
    const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event);

    switch (type) {
    case MessageComponentType::Verification: {
        return {MessageComponent{MessageComponentType::Verification, QString(), {}}};
    }
    case MessageComponentType::Text: {
        return TextHandler().textComponents(EventHandler::rawMessageBody(*event),
                                            EventHandler::messageBodyInputFormat(*event),
                                            m_room,
                                            event,
                                            roomMessageEvent ? roomMessageEvent->isReplaced() : false);
    }
    case MessageComponentType::File: {
        QList<MessageComponent> components;
        components += MessageComponent{MessageComponentType::File, {}, EventHandler::mediaInfo(m_room, event)};
        auto body = EventHandler::rawMessageBody(*event);
        if (!body.isEmpty()) {
            components += TextHandler().textComponents(body,
                                                       EventHandler::messageBodyInputFormat(*event),
                                                       m_room,
                                                       event,
                                                       roomMessageEvent ? roomMessageEvent->isReplaced() : false);
        }
        return components;
    }
    case MessageComponentType::Image:
    case MessageComponentType::Audio:
    case MessageComponentType::Video: {
        QList<MessageComponent> components = {MessageComponent{type, EventHandler::richBody(m_room, event), EventHandler::mediaInfo(m_room, event)}};

        if (!event->is<StickerEvent>() && roomMessageEvent) {
            const auto fileContent = roomMessageEvent->get<EventContent::FileContentBase>();
            if (fileContent != nullptr) {
                const auto fileInfo = fileContent->commonInfo();
                const auto body = EventHandler::rawMessageBody(*roomMessageEvent);
                // Do not attach the description to the image, if it's the same as the original filename.
                if (fileInfo.originalName != body) {
                    components += TextHandler().textComponents(body,
                                                               EventHandler::messageBodyInputFormat(*roomMessageEvent),
                                                               m_room,
                                                               roomMessageEvent,
                                                               roomMessageEvent->isReplaced());
                }
            }
        }
        return components;
    }
    case MessageComponentType::Location:
        return {MessageComponent{type,
                                 EventHandler::plainBody(m_room, event),
                                 {
                                     {u"latitude"_s, EventHandler::latitude(event)},
                                     {u"longitude"_s, EventHandler::longitude(event)},
                                     {u"asset"_s, EventHandler::locationAssetType(event)},
                                 }}};
    default:
        return {MessageComponent{type, QString(), {}}};
    }
}

void EventMessageContentModel::updateItineraryModel()
{
    if (!hasComponentType(MessageComponentType::File) || !m_room) {
        return;
    }

    const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(m_room->getEvent(m_eventId).first);
    if (!roomMessageEvent || !roomMessageEvent->has<EventContent::FileContent>()) {
        return;
    }

    auto filePath = m_room->cachedFileTransferInfo(roomMessageEvent).localPath;
    if (filePath.isEmpty() && m_itineraryModel != nullptr) {
        delete m_itineraryModel;
        m_itineraryModel = nullptr;
    } else if (!filePath.isEmpty()) {
        if (m_itineraryModel == nullptr) {
            m_itineraryModel = new ItineraryModel(this);
            connect(m_itineraryModel, &ItineraryModel::loaded, this, [this]() {
                if (m_itineraryModel->rowCount() == 0) {
                    m_emptyItinerary = true;
                    m_itineraryModel->deleteLater();
                    m_itineraryModel = nullptr;
                }
                Q_EMIT itineraryUpdated();
            });
            connect(m_itineraryModel, &ItineraryModel::loadErrorOccurred, this, [this]() {
                m_emptyItinerary = true;
                m_itineraryModel->deleteLater();
                m_itineraryModel = nullptr;
                Q_EMIT itineraryUpdated();
            });
        }
        m_itineraryModel->setPath(filePath.toString());
    }
}

void EventMessageContentModel::updateReactionModel()
{
    if (m_reactionModel && m_reactionModel->rowCount() > 0) {
        return;
    }

    if (m_reactionModel == nullptr) {
        m_reactionModel = new ReactionModel(this, m_eventId, m_room);
        connect(m_reactionModel, &ReactionModel::reactionsUpdated, this, &EventMessageContentModel::updateReactionModel);
    }

    if (m_reactionModel->rowCount() <= 0) {
        m_reactionModel->disconnect(this);
        m_reactionModel->deleteLater();
        m_reactionModel = nullptr;
    }

    if (m_reactionModel && m_components.last().type != MessageComponentType::Reaction) {
        beginInsertRows({}, rowCount(), rowCount());
        m_components += MessageComponent{MessageComponentType::Reaction, QString(), {}};
        endInsertRows();
    } else if (rowCount() > 0 && m_components.last().type == MessageComponentType::Reaction) {
        beginRemoveRows({}, rowCount() - 1, rowCount() - 1);
        m_components.removeLast();
        endRemoveRows();
    }
}

ThreadModel *EventMessageContentModel::modelForThread(const QString &threadRootId)
{
    return ContentProvider::self().modelForThread(m_room, threadRootId);
}

void EventMessageContentModel::setThreadsEnabled(bool enableThreads)
{
    m_threadsEnabled = enableThreads;
}

#include "moc_eventmessagecontentmodel.cpp"
