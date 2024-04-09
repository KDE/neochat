// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecontentmodel.h"
#include "neochatconfig.h"

#include <QImageReader>

#include <Quotient/events/redactionevent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/room.h>

#include <KLocalizedString>

#ifndef Q_OS_ANDROID
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#endif

#include "chatbarcache.h"
#include "enums/messagecomponenttype.h"
#include "eventhandler.h"
#include "filetype.h"
#include "itinerarymodel.h"
#include "linkpreviewer.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "texthandler.h"

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
                    m_components[0].type = MessageComponentType::Reply;
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
                updateComponents();
                Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferFailed, this, [this](const QString &eventId) {
            if (m_event != nullptr && eventId == m_event->id()) {
                updateComponents();
                Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
            }
        });
        connect(m_room->editCache(), &ChatBarCache::relationIdChanged, this, [this](const QString &oldEventId, const QString &newEventId) {
            if (m_event != nullptr && (oldEventId == m_event->id() || newEventId == m_event->id())) {
                // HACK: Because DelegateChooser can't switch the delegate on dataChanged it has to think there is a new delegate.
                beginResetModel();
                updateComponents(newEventId == m_event->id());
                endResetModel();
            }
        });
        connect(m_room, &NeoChatRoom::urlPreviewEnabledChanged, this, &MessageContentModel::updateLinkPreviewer);
        connect(NeoChatConfig::self(), &NeoChatConfig::ShowLinkPreviewChanged, this, &MessageContentModel::updateLinkPreviewer);
    }

    updateLinkPreviewer();
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
    const auto component = m_components[index.row()];

    if (role == DisplayRole) {
        if (m_event->isRedacted()) {
            auto reason = m_event->redactedBecause()->reason();
            return (reason.isEmpty()) ? i18n("<i>[This message was deleted]</i>")
                                      : i18n("<i>[This message was deleted: %1]</i>", m_event->redactedBecause()->reason());
        }
        if (!component.content.isEmpty()) {
            return component.content;
        }
        return eventHandler.getRichBody();
    }
    if (role == ComponentTypeRole) {
        return component.type;
    }
    if (role == ComponentAttributesRole) {
        return component.attributes;
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
    if (role == ItineraryModelRole) {
        return QVariant::fromValue<ItineraryModel *>(m_itineraryModel);
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
    roles[ComponentAttributesRole] = "componentAttributes";
    roles[EventIdRole] = "eventId";
    roles[AuthorRole] = "author";
    roles[MediaInfoRole] = "mediaInfo";
    roles[FileTransferInfoRole] = "fileTransferInfo";
    roles[ItineraryModelRole] = "itineraryModel";
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

void MessageContentModel::updateComponents(bool isEditing)
{
    beginResetModel();
    m_components.clear();

    if (eventCast<const Quotient::RoomMessageEvent>(m_event)
        && eventCast<const Quotient::RoomMessageEvent>(m_event)->rawMsgtype() == QStringLiteral("m.key.verification.request")) {
        m_components += MessageComponent{MessageComponentType::Verification, QString(), {}};
    } else {
        EventHandler eventHandler(m_room, m_event);
        if (eventHandler.hasReply()) {
            if (m_room->findInTimeline(eventHandler.getReplyId()) == m_room->historyEdge()) {
                m_components += MessageComponent{MessageComponentType::ReplyLoad, QString(), {}};
                m_room->loadReply(m_event->id(), eventHandler.getReplyId());
            } else {
                m_components += MessageComponent{MessageComponentType::Reply, QString(), {}};
            }
        }

        if (isEditing) {
            m_components += MessageComponent{MessageComponentType::Edit, QString(), {}};
        } else if (m_event->isRedacted()) {
            m_components += MessageComponent{MessageComponentType::Text, QString(), {}};
        } else {
            if (eventHandler.messageComponentType() == MessageComponentType::Text) {
                const auto event = eventCast<const Quotient::RoomMessageEvent>(m_event);
                auto body = EventHandler::rawMessageBody(*event);
                m_components.append(TextHandler().textComponents(body, EventHandler::messageBodyInputFormat(*event), m_room, event, event->isReplaced()));
            } else if (eventHandler.messageComponentType() == MessageComponentType::File) {
                m_components += MessageComponent{MessageComponentType::File, QString(), {}};
                if (m_emptyItinerary) {
                    Quotient::FileTransferInfo fileTransferInfo;
                    if (auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
                        if (event->hasFileContent()) {
                            fileTransferInfo = m_room->fileTransferInfo(event->id());
                        }
                    }
                    if (auto event = eventCast<const Quotient::StickerEvent>(m_event)) {
                        fileTransferInfo = m_room->fileTransferInfo(event->id());
                    }

#ifndef Q_OS_ANDROID
                    KSyntaxHighlighting::Repository repository;
                    const auto definitionForFile = repository.definitionForFileName(fileTransferInfo.localPath.toString());
                    if (definitionForFile.isValid() || QFileInfo(fileTransferInfo.localPath.path()).suffix() == QStringLiteral("txt")) {
                        QFile file(fileTransferInfo.localPath.path());
                        file.open(QIODevice::ReadOnly);
                        m_components += MessageComponent{MessageComponentType::Code,
                                                         QString::fromStdString(file.readAll().toStdString()),
                                                         {{QStringLiteral("class"), definitionForFile.name()}}};
                    }
#endif

                    if (FileType::instance().fileHasImage(fileTransferInfo.localPath)) {
                        QImageReader reader(fileTransferInfo.localPath.path());
                        m_components += MessageComponent{MessageComponentType::Pdf, QString(), {{QStringLiteral("size"), reader.size()}}};
                    }
                } else {
                    updateItineraryModel();
                    if (m_itineraryModel != nullptr) {
                        m_components += MessageComponent{MessageComponentType::Itinerary, QString(), {}};
                    }
                }
            } else {
                m_components += MessageComponent{eventHandler.messageComponentType(), QString(), {}};
            }
        }

        if (m_linkPreviewer != nullptr) {
            if (m_linkPreviewer->loaded()) {
                m_components += MessageComponent{MessageComponentType::LinkPreview, QString(), {}};
            } else {
                m_components += MessageComponent{MessageComponentType::LinkPreviewLoad, QString(), {}};
            }
        }
    }

    endResetModel();
}

void MessageContentModel::updateLinkPreviewer()
{
    if (m_room == nullptr || m_event == nullptr) {
        if (m_linkPreviewer != nullptr) {
            m_linkPreviewer->disconnect(this);
            m_linkPreviewer = nullptr;
            updateComponents();
        }
        return;
    }
    if (!m_room->urlPreviewEnabled()) {
        if (m_linkPreviewer != nullptr) {
            m_linkPreviewer->disconnect(this);
            m_linkPreviewer = nullptr;
            updateComponents();
        }
        return;
    }

    if (const auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
        if (LinkPreviewer::hasPreviewableLinks(event)) {
            m_linkPreviewer = dynamic_cast<NeoChatConnection *>(m_room->connection())->previewerForLink(LinkPreviewer::linkPreview(event));
            updateComponents();

            if (m_linkPreviewer != nullptr) {
                connect(m_linkPreviewer, &LinkPreviewer::loadedChanged, [this]() {
                    if (m_linkPreviewer != nullptr && m_linkPreviewer->loaded()) {
                        // HACK: Because DelegateChooser can't switch the delegate on dataChanged it has to think there is a new delegate.
                        beginResetModel();
                        m_components[m_components.size() - 1].type = MessageComponentType::LinkPreview;
                        endResetModel();
                    }
                });
            }
        }
    }
}

void MessageContentModel::updateItineraryModel()
{
    if (m_room == nullptr || m_event == nullptr) {
        return;
    }

    if (auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
        if (event->hasFileContent()) {
            auto filePath = m_room->fileTransferInfo(event->id()).localPath;
            if (filePath.isEmpty() && m_itineraryModel != nullptr) {
                delete m_itineraryModel;
                m_itineraryModel = nullptr;
            } else if (!filePath.isEmpty()) {
                if (m_itineraryModel == nullptr) {
                    m_itineraryModel = new ItineraryModel(this);
                    connect(m_itineraryModel, &ItineraryModel::loaded, this, [this]() {
                        if (m_itineraryModel->rowCount() == 0) {
                            m_itineraryModel->deleteLater();
                            m_itineraryModel = nullptr;
                            m_emptyItinerary = true;
                            updateComponents();
                        }
                    });
                    connect(m_itineraryModel, &ItineraryModel::loadErrorOccurred, this, [this]() {
                        m_itineraryModel->deleteLater();
                        m_itineraryModel = nullptr;
                        m_emptyItinerary = true;
                        updateComponents();
                    });
                }
                m_itineraryModel->setPath(filePath.toString());
            }
        }
    }
}
