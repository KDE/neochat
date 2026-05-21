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

#include "block.h"
#include "chatbarcache.h"
#include "contentprovider.h"
#include "enums/blocktype.h"
#include "eventhandler.h"
#include "models/reactionmodel.h"
#include "neochatdatetime.h"
#include "neochatroom.h"
#include "texthandler.h"

using namespace Quotient;

EventMessageContentModel::EventMessageContentModel(NeoChatRoom *room, const QString &eventId, bool isReply, bool isPending, MessageContentModel *parent)
    : MessageContentModel(room, eventId, parent)
    , m_currentState(isPending ? Pending : Unknown)
    , m_isReply(isReply)
{
    initializeModel();
}

void EventMessageContentModel::initializeModel()
{
    Q_ASSERT(m_room != nullptr);
    Q_ASSERT(!m_eventId.isEmpty());

    connect(this, &MessageContentModel::componentsUpdated, this, &EventMessageContentModel::checkFilePreview);
    connect(m_room, &NeoChatRoom::fileTransferCompleted, this, [this](const QString &eventId) {
        if (eventId == m_eventId) {
            m_fileChecked = false;
            checkFilePreview();
        }
    });
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
    connect(m_room, &Room::updatedEvent, this, [this](const QString &eventId) {
        if (eventId == m_eventId) {
            updateReactionModel();
        }
    });
#if Quotient_VERSION_MINOR > 9
    connect(m_room, &Room::newThread, this, [this](const QString &newThread) {
        if (newThread == m_eventId) {
            resetContent();
        }
    });
#elif Quotient_VERSION_MINOR == 9 && Quotient_VERSION_PATCH >= 4
    connect(m_room, &Room::newThread, this, [this](const Thread &newThread) {
        if (newThread.threadRootId == m_eventId) {
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

Blocks::Block *EventMessageContentModel::unavailableBlock()
{
    const auto theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));

    QString disabledTextColor;
    if (theme != nullptr) {
        disabledTextColor = theme->disabledTextColor().name();
    } else {
        disabledTextColor = u"#000000"_s;
    }

    return new Blocks::TextBlock(
        Blocks::Text,
        QTextDocumentFragment::fromHtml(
            u"<span style=\"color:%1\">"_s.arg(disabledTextColor)
            + i18nc("@info", "This message was either not found, you do not have permission to view it, or it was sent by an ignored user") + u"</span>"_s),
        false,
        this);
}

void EventMessageContentModel::resetModel()
{
    beginResetModel();
    m_components.clear();
    if (m_replyModel) {
        m_replyModel->disconnect(this);
        m_replyModel->deleteLater();
    }

    if (m_room->connection()->isIgnored(authorId()) || m_currentState == UnAvailable) {
        m_components.push_back(unavailableBlock());
        endResetModel();
        return;
    }

    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr) {
        m_components.push_back(
            new Blocks::BasicTextBlock(Blocks::Loading, m_isReply ? i18nc("@info", "Loading reply…") : i18nc("@info Loading this message", "Loading…"), this));
        endResetModel();
        return;
    }

    m_components.push_back(new Blocks::Block(Blocks::Author, this));

    auto components = messageContentComponents();
    m_components.insert(m_components.end(), std::make_move_iterator(components.begin()), std::make_move_iterator(components.end()));
    endResetModel();

    updateReactionModel();

    Q_EMIT componentsUpdated();
    // We need QML to re-evaluate author (for example, reply colors) if it was previously null.
    Q_EMIT authorChanged();
}

void EventMessageContentModel::resetContent(bool isEditing, bool isThreading)
{
    const auto startIt = m_components.begin() + (m_components[0]->type() == Blocks::Author ? 1 : 0);
    const auto startRow = std::distance(m_components.begin(), startIt);
    beginRemoveRows({}, startRow, rowCount() - 1);
    m_components.erase(startIt, m_components.end());
    endRemoveRows();
    if (m_replyModel) {
        m_replyModel->disconnect(this);
        m_replyModel->deleteLater();
    }

    auto newComponents = messageContentComponents(isEditing, isThreading);
    if (newComponents.size() == 0) {
        return;
    }
    beginInsertRows({}, startRow, startRow + newComponents.size() - 1);
    m_components.insert(startIt, std::make_move_iterator(newComponents.begin()), std::make_move_iterator(newComponents.end()));
    endInsertRows();

    updateReactionModel();

    Q_EMIT componentsUpdated();
}

Blocks::BlockPtrs EventMessageContentModel::messageContentComponents(bool isEditing, bool isThreading)
{
    const auto [event, _] = m_room->getEvent(m_eventId);
    if (!event) {
        return {};
    }

    Blocks::BlockPtrs blocks;

    if (!m_isReply && event->isReply()) {
        blocks.push_back(new Blocks::ReplyBlock(Blocks::Reply, event->replyEventId(), this));
        m_replyModel = new EventMessageContentModel(m_room, event->replyEventId(), true, false, this);
    }

    if (isEditing) {
        blocks.push_back(new Blocks::ChatBarBlock(Blocks::ChatBar, true, {}, this));
    } else {
        blocks.insert_range(blocks.end(), EventHandler::blocksForEvent(m_room, event, this));
    }

    const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event);
    // If the event is already threaded the ThreadModel will handle displaying a chat bar.
    if (isThreading && roomMessageEvent && !(roomMessageEvent->isThreaded() || m_room->threads().contains(roomMessageEvent->id()))) {
        blocks.push_back(new Blocks::ChatBarBlock(Blocks::ChatBar, false, m_eventId, this));
    }

    return blocks;
}

void EventMessageContentModel::checkFilePreview()
{
    if (m_loader) {
        if (m_loader->loaded()) {
            insertFIlePreview();
            return;
        }
    }
    if (m_fileChecked) {
        return;
    }
    m_fileChecked = true;
    auto it = std::ranges::find_if(m_components, [](Blocks::Block *block) {
        return block->type() == Blocks::File;
    });
    if (it == m_components.end()) {
        return;
    }
    m_loader = new Blocks::FilePreviewBlockLoader(this, m_room->cachedFileTransferInfo(m_eventId).localPath);
    connect(m_loader, &Blocks::FilePreviewBlockLoader::blockAvailable, this, [this]() {
        insertFIlePreview();
    });
    connect(m_loader, &Blocks::FilePreviewBlockLoader::blockUnavailable, this, [this]() {
        m_loader->deleteLater();
        m_loader = nullptr;
    });
}

void EventMessageContentModel::insertFIlePreview()
{
    if (!m_loader || !m_loader->loaded()) {
        return;
    }
    auto it = std::ranges::find_if(m_components, [](Blocks::Block *block) {
        return block->type() == Blocks::File;
    });
    if (it == m_components.end()) {
        return;
    }
    const auto insertIt = it + 1;
    const auto insertRow = std::distance(m_components.begin(), insertIt);
    beginInsertRows({}, insertRow, insertRow);
    m_components.insert(insertIt, m_loader->previewBlock());
    endInsertRows();
}

void EventMessageContentModel::updateReactionModel()
{
    if (hasComponentType(Blocks::Reaction)) {
        bool hasReactions = false;
        forEachComponentOfType(Blocks::Reaction, [&hasReactions](Blocks::BlockPtrsIt it) {
            if (const auto reactionBlock = dynamic_cast<Blocks::ReactionBlock *>(*it)) {
                hasReactions = reactionBlock->model()->rowCount() > 0;
            }
            return ++it;
        });
        if (hasReactions) {
            return;
        }
    }

    if (m_components.back()->type() != Blocks::Reaction) {
        auto reactionBlock = new Blocks::ReactionBlock(Blocks::Reaction, m_room, m_eventId, this);
        if (reactionBlock->model()->rowCount() > 0) {
            beginInsertRows({}, rowCount(), rowCount());
            m_components.push_back(reactionBlock);
            endInsertRows();
        }
    } else if (rowCount() > 0 && m_components.back()->type() == Blocks::Reaction) {
        beginRemoveRows({}, rowCount() - 1, rowCount() - 1);
        m_components.erase(--m_components.end());
        endRemoveRows();
    }
}

void EventMessageContentModel::replyInThread()
{
    if (hasComponentType(Blocks::ThreadBody)) {
        if (const auto threadModel = modelForThread(m_eventId)) {
            threadModel->setReplying(true);
        }
        return;
    }
    resetContent(false, true);
}

void EventMessageContentModel::cancelReplyInThread()
{
    if (hasComponentType(Blocks::ThreadBody)) {
        if (const auto threadModel = modelForThread(m_eventId)) {
            threadModel->setReplying(false);
        }
        return;
    }
    if (hasComponentType(Blocks::ChatBar)) {
        forEachComponentOfType(Blocks::ChatBar, [this](Blocks::BlockPtrsIt it) {
            beginRemoveRows({}, std::distance(m_components.begin(), it), std::distance(m_components.begin(), it));
            it = m_components.erase(it);
            endRemoveRows();
            return it;
        });
    }
}

ThreadModel *EventMessageContentModel::modelForThread(const QString &threadRootId)
{
    return ContentProvider::self().modelForThread(m_room, threadRootId);
}

#include "moc_eventmessagecontentmodel.cpp"
