// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecontentmodel.h"

#include <Quotient/events/redactionevent.h>
#include <Quotient/events/stickerevent.h>

#include <KLocalizedString>

#include "chatbarcache.h"
#include "enums/messagecomponenttype.h"
#include "eventhandler.h"
#include "linkpreviewer.h"
#include "neochatroom.h"

MessageContentModel::MessageContentModel(const Quotient::RoomEvent *event, NeoChatRoom *room)
    : QAbstractListModel(nullptr)
    , m_room(room)
    , m_event(event)
{
    if (m_room != nullptr) {
        connect(m_room, &NeoChatRoom::pendingEventAboutToMerge, this, [this](Quotient::RoomEvent *serverEvent) {
            if (m_room != nullptr && m_event != nullptr) {
                if (m_event->id() == serverEvent->id()) {
                    beginResetModel();
                    m_event = serverEvent;
                    endResetModel();
                }
            }
        });
        connect(m_room, &NeoChatRoom::replacedEvent, this, [this](const Quotient::RoomEvent *newEvent) {
            if (m_room != nullptr && m_event != nullptr) {
                if (m_event->id() == newEvent->id()) {
                    beginResetModel();
                    m_event = newEvent;
                    endResetModel();
                }
            }
        });
        connect(m_room, &NeoChatRoom::replyLoaded, this, [this](const QString &eventId, const QString &replyId) {
            Q_UNUSED(eventId)
            if (m_event != nullptr && m_room != nullptr) {
                const auto eventHandler = EventHandler(m_room, m_event);
                if (replyId == eventHandler.getReplyId()) {
                    // HACK: Because DelegateChooser can't switch the delegate on dataChanged it has to think there is a new delegate.
                    beginResetModel();
                    m_components[0] = MessageComponentType::Reply;
                    endResetModel();
                }
            }
        });
        connect(m_room, &NeoChatRoom::newFileTransfer, this, [this](const QString &eventId) {
            if (m_event != nullptr && eventId == m_event->id()) {
                Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferProgress, this, [this](const QString &eventId) {
            if (m_event != nullptr && eventId == m_event->id()) {
                Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferCompleted, this, [this](const QString &eventId) {
            if (m_event != nullptr && eventId == m_event->id()) {
                Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferFailed, this, [this](const QString &eventId) {
            if (m_event != nullptr && eventId == m_event->id()) {
                Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
            }
        });
        connect(m_room->editCache(), &ChatBarCache::relationIdChanged, this, [this](const QString &oldEventId, const QString &newEventId) {
            if (m_event != nullptr && (oldEventId == m_event->id() || newEventId == m_event->id())) {
                // HACK: Because DelegateChooser can't switch the delegate on dataChanged it has to think there is a new delegate.
                beginResetModel();
                endResetModel();
            }
        });
    }

    if (const auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
        if (LinkPreviewer::hasPreviewableLinks(event)) {
            m_linkPreviewer = new LinkPreviewer(m_room, event, this);

            connect(m_linkPreviewer, &LinkPreviewer::loadedChanged, [this]() {
                if (m_linkPreviewer->loaded()) {
                    // HACK: Because DelegateChooser can't switch the delegate on dataChanged it has to think there is a new delegate.
                    beginResetModel();
                    m_components[m_components.size() - 1] = MessageComponentType::LinkPreview;
                    endResetModel();
                }
            });
        }
    }

    updateComponents();
}

static LinkPreviewer *emptyLinkPreview = new LinkPreviewer;

QVariant MessageContentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= rowCount()) {
        qDebug() << "MessageContentModel, something's wrong: index.row() >= rowCount()";
        return {};
    }

    EventHandler eventHandler(m_room, m_event);

    if (role == DisplayRole) {
        if (m_event->isRedacted()) {
            auto reason = m_event->redactedBecause()->reason();
            return (reason.isEmpty()) ? i18n("<i>[This message was deleted]</i>")
                                      : i18n("<i>[This message was deleted: %1]</i>", m_event->redactedBecause()->reason());
        }
        return eventHandler.getRichBody();
    }
    if (role == ComponentTypeRole) {
        const auto component = m_components[index.row()];
        if (component == MessageComponentType::Text && !m_event->id().isEmpty() && m_room->editCache()->editId() == m_event->id()) {
            return MessageComponentType::Edit;
        }
        return component;
    }
    if (role == EventIdRole) {
        return eventHandler.getId();
    }
    if (role == AuthorRole) {
        return eventHandler.getAuthor(false);
    }
    if (role == MediaInfoRole) {
        return eventHandler.getMediaInfo();
    }
    if (role == FileTransferInfoRole) {
        if (auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
            if (event->hasFileContent()) {
                return QVariant::fromValue(m_room->fileTransferInfo(event->id()));
            }
        }
        if (auto event = eventCast<const Quotient::StickerEvent>(m_event)) {
            return QVariant::fromValue(m_room->fileTransferInfo(event->id()));
        }
    }
    if (role == LatitudeRole) {
        return eventHandler.getLatitude();
    }
    if (role == LongitudeRole) {
        return eventHandler.getLongitude();
    }
    if (role == AssetRole) {
        return eventHandler.getLocationAssetType();
    }
    if (role == PollHandlerRole) {
        return QVariant::fromValue<PollHandler *>(m_room->poll(m_event->id()));
    }
    if (role == IsReplyRole) {
        return eventHandler.hasReply();
    }
    if (role == ReplyComponentType) {
        return eventHandler.replyMessageComponentType();
    }
    if (role == ReplyEventIdRole) {
        return eventHandler.getReplyId();
    }
    if (role == ReplyAuthorRole) {
        return eventHandler.getReplyAuthor();
    }
    if (role == ReplyDisplayRole) {
        return eventHandler.getReplyRichBody();
    }
    if (role == ReplyMediaInfoRole) {
        return eventHandler.getReplyMediaInfo();
    }
    if (role == LinkPreviewerRole) {
        if (m_linkPreviewer != nullptr) {
            return QVariant::fromValue<LinkPreviewer *>(m_linkPreviewer);
        } else {
            return QVariant::fromValue<LinkPreviewer *>(emptyLinkPreview);
        }
    }

    return {};
}

int MessageContentModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_components.size();
}

QHash<int, QByteArray> MessageContentModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[DisplayRole] = "display";
    roles[ComponentTypeRole] = "componentType";
    roles[EventIdRole] = "eventId";
    roles[AuthorRole] = "author";
    roles[MediaInfoRole] = "mediaInfo";
    roles[FileTransferInfoRole] = "fileTransferInfo";
    roles[LatitudeRole] = "latitude";
    roles[LongitudeRole] = "longitude";
    roles[AssetRole] = "asset";
    roles[PollHandlerRole] = "pollHandler";
    roles[IsReplyRole] = "isReply";
    roles[ReplyComponentType] = "replyComponentType";
    roles[ReplyEventIdRole] = "replyEventId";
    roles[ReplyAuthorRole] = "replyAuthor";
    roles[ReplyDisplayRole] = "replyDisplay";
    roles[ReplyMediaInfoRole] = "replyMediaInfo";
    roles[LinkPreviewerRole] = "linkPreviewer";
    return roles;
}

void MessageContentModel::updateComponents()
{
    beginResetModel();
    m_components.clear();

    EventHandler eventHandler(m_room, m_event);
    if (eventHandler.hasReply()) {
        if (m_room->findInTimeline(eventHandler.getReplyId()) == m_room->historyEdge()) {
            m_components += MessageComponentType::ReplyLoad;
            m_room->loadReply(m_event->id(), eventHandler.getReplyId());
        } else {
            m_components += MessageComponentType::Reply;
        }
    }

    m_components += eventHandler.messageComponentType();

    if (m_linkPreviewer != nullptr) {
        if (m_linkPreviewer->loaded()) {
            m_components += MessageComponentType::LinkPreview;
        } else {
            m_components += MessageComponentType::LinkPreviewLoad;
        }
    }

    endResetModel();
}
