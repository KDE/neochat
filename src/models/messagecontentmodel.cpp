// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecontentmodel.h"
#include "neochatconfig.h"
#include "neochatroommember.h"

#include <QImageReader>

#include <Quotient/events/eventcontent.h>
#include <Quotient/events/redactionevent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/stickerevent.h>

#include <KLocalizedString>
#include <Quotient/qt_connection_util.h>

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

using namespace Quotient;

MessageContentModel::MessageContentModel(NeoChatRoom *room, const Quotient::RoomEvent *event, bool isReply, bool isPending)
    : QAbstractListModel(nullptr)
    , m_room(room)
    , m_eventId(event != nullptr ? event->id() : QString())
    , m_eventSenderId(event != nullptr ? event->senderId() : QString())
    , m_isPending(isPending)
    , m_isReply(isReply)
{
    intiializeEvent(event);
    initializeModel();
}

MessageContentModel::MessageContentModel(NeoChatRoom *room, const QString &eventId, bool isReply, bool isPending)
    : QAbstractListModel(nullptr)
    , m_room(room)
    , m_eventId(eventId)
    , m_isPending(isPending)
    , m_isReply(isReply)
{
    initializeModel();
}

void MessageContentModel::initializeModel()
{
    Q_ASSERT(m_room != nullptr);
    // Allow making a model for an event that is being downloaded but will appear later
    // e.g. a reply, but we need an ID to know when it has arrived.
    Q_ASSERT(!m_eventId.isEmpty());

    Quotient::connectUntil(m_room.get(), &NeoChatRoom::extraEventLoaded, this, [this](const QString &eventId) {
        if (m_room != nullptr) {
            if (eventId == m_eventId) {
                m_event = loadEvent<RoomEvent>(m_room->getEvent(eventId)->fullJson());
                Q_EMIT eventUpdated();
                updateReplyModel();
                resetContent();
                return true;
            }
        }
        return false;
    });

    if (m_event == nullptr) {
        m_room->downloadEventFromServer(m_eventId);
    }

    connect(m_room, &NeoChatRoom::pendingEventAboutToMerge, this, [this](Quotient::RoomEvent *serverEvent) {
        if (m_room != nullptr && m_event != nullptr) {
            if (m_eventId == serverEvent->id()) {
                beginResetModel();
                m_isPending = false;
                intiializeEvent(serverEvent);
                endResetModel();
            }
        }
    });
    connect(m_room, &NeoChatRoom::replacedEvent, this, [this](const Quotient::RoomEvent *newEvent) {
        if (m_room != nullptr && m_event != nullptr) {
            if (m_eventId == newEvent->id()) {
                beginResetModel();
                intiializeEvent(newEvent);
                endResetModel();
            }
        }
    });
    connect(m_room, &NeoChatRoom::newFileTransfer, this, [this](const QString &eventId) {
        if (m_event != nullptr && eventId == m_eventId) {
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
        }
    });
    connect(m_room, &NeoChatRoom::fileTransferProgress, this, [this](const QString &eventId) {
        if (m_event != nullptr && eventId == m_eventId) {
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
        }
    });
    connect(m_room, &NeoChatRoom::fileTransferCompleted, this, [this](const QString &eventId) {
        if (m_room != nullptr && m_event != nullptr && eventId == m_eventId) {
            resetContent();
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
        }
    });
    connect(m_room, &NeoChatRoom::fileTransferFailed, this, [this](const QString &eventId) {
        if (m_event != nullptr && eventId == m_eventId) {
            resetContent();
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
        }
    });
    connect(m_room->editCache(), &ChatBarCache::relationIdChanged, this, [this](const QString &oldEventId, const QString &newEventId) {
        if (m_event != nullptr && (oldEventId == m_eventId || newEventId == m_eventId)) {
            // HACK: Because DelegateChooser can't switch the delegate on dataChanged it has to think there is a new delegate.
            beginResetModel();
            resetContent(newEventId == m_eventId);
            endResetModel();
        }
    });
    connect(m_room, &NeoChatRoom::urlPreviewEnabledChanged, this, [this]() {
        resetContent();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowLinkPreviewChanged, this, [this]() {
        resetContent();
    });
    connect(m_room, &Room::memberNameUpdated, this, [this](RoomMember member) {
        if (m_room != nullptr && m_event != nullptr) {
            if (m_eventSenderId.isEmpty() || m_eventSenderId == member.id()) {
                Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole});
            }
        }
    });
    connect(m_room, &Room::memberAvatarUpdated, this, [this](RoomMember member) {
        if (m_room != nullptr && m_event != nullptr) {
            if (m_eventSenderId.isEmpty() || m_eventSenderId == member.id()) {
                Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole});
            }
        }
    });

    if (m_event != nullptr) {
        updateReplyModel();
    }
    resetModel();
}

void MessageContentModel::intiializeEvent(const QString &eventId)
{
    const auto newEvent = m_room->getEvent(eventId);
    if (newEvent != nullptr) {
        intiializeEvent(newEvent);
    }
}

void MessageContentModel::intiializeEvent(const Quotient::RoomEvent *event)
{
    m_event = loadEvent<RoomEvent>(event->fullJson());
    auto senderId = event->senderId();
    // A pending event might not have a sender ID set yet but in that case it must
    // be the local member.
    if (senderId.isEmpty()) {
        senderId = m_room->localMember().id();
    }
    if (m_eventSenderObject == nullptr) {
        m_eventSenderObject = std::unique_ptr<NeochatRoomMember>(new NeochatRoomMember(m_room, senderId));
    }
    Q_EMIT eventUpdated();
}

bool MessageContentModel::showAuthor() const
{
    return m_showAuthor;
}

void MessageContentModel::setShowAuthor(bool showAuthor)
{
    if (showAuthor == m_showAuthor) {
        return;
    }
    m_showAuthor = showAuthor;

    if (m_event != nullptr) {
        if (showAuthor) {
            beginInsertRows({}, 0, 0);
            m_components.prepend(MessageComponent{MessageComponentType::Author, QString(), {}});
            endInsertRows();
        } else {
            beginRemoveRows({}, 0, 0);
            m_components.remove(0, 1);
            endRemoveRows();
        }
    }
    Q_EMIT showAuthorChanged();
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

    EventHandler eventHandler(m_room, m_event.get());
    const auto component = m_components[index.row()];

    if (role == DisplayRole) {
        if (component.type == MessageComponentType::Loading && m_isReply) {
            return i18n("Loading reply");
        }
        if (m_event == nullptr) {
            return QString();
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
    if (role == TimeRole) {
        const auto pendingIt = std::find_if(m_room->pendingEvents().cbegin(), m_room->pendingEvents().cend(), [this](const PendingEventItem &pendingEvent) {
            return m_event->transactionId() == pendingEvent->transactionId();
        });

        auto lastUpdated = pendingIt == m_room->pendingEvents().cend() ? QDateTime() : pendingIt->lastUpdated();
        return eventHandler.getTime(m_isPending, lastUpdated);
    }
    if (role == TimeStringRole) {
        const auto pendingIt = std::find_if(m_room->pendingEvents().cbegin(), m_room->pendingEvents().cend(), [this](const PendingEventItem &pendingEvent) {
            return m_event->transactionId() == pendingEvent->transactionId();
        });

        auto lastUpdated = pendingIt == m_room->pendingEvents().cend() ? QDateTime() : pendingIt->lastUpdated();
        return eventHandler.getTimeString(QStringLiteral("hh:mm"), m_isPending, lastUpdated);
    }
    if (role == AuthorRole) {
        return QVariant::fromValue<NeochatRoomMember *>(m_eventSenderObject.get());
    }
    if (role == MediaInfoRole) {
        return eventHandler.getMediaInfo();
    }
    if (role == FileTransferInfoRole) {
        return QVariant::fromValue(m_room->cachedFileTransferInfo(m_event.get()));
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
        return QVariant::fromValue<PollHandler *>(m_room->poll(m_eventId));
    }
    if (role == IsReplyRole) {
        return eventHandler.hasReply();
    }
    if (role == ReplyEventIdRole) {
        return eventHandler.getReplyId();
    }
    if (role == ReplyAuthorRole) {
        return QVariant::fromValue(eventHandler.getReplyAuthor());
    }
    if (role == ReplyContentModelRole) {
        return QVariant::fromValue<MessageContentModel *>(m_replyModel);
    }
    if (role == LinkPreviewerRole) {
        if (component.type == MessageComponentType::LinkPreview) {
            return QVariant::fromValue<LinkPreviewer *>(
                dynamic_cast<NeoChatConnection *>(m_room->connection())->previewerForLink(component.attributes["link"_ls].toUrl()));
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
    roles[TimeRole] = "time";
    roles[TimeStringRole] = "timeString";
    roles[AuthorRole] = "author";
    roles[MediaInfoRole] = "mediaInfo";
    roles[FileTransferInfoRole] = "fileTransferInfo";
    roles[ItineraryModelRole] = "itineraryModel";
    roles[LatitudeRole] = "latitude";
    roles[LongitudeRole] = "longitude";
    roles[AssetRole] = "asset";
    roles[PollHandlerRole] = "pollHandler";
    roles[IsReplyRole] = "isReply";
    roles[ReplyEventIdRole] = "replyEventId";
    roles[ReplyAuthorRole] = "replyAuthor";
    roles[ReplyContentModelRole] = "replyContentModel";
    roles[LinkPreviewerRole] = "linkPreviewer";
    return roles;
}

void MessageContentModel::resetModel()
{
    beginResetModel();
    m_components.clear();

    if (m_event == nullptr) {
        m_components += MessageComponent{MessageComponentType::Loading, QString(), {}};
        endResetModel();
        return;
    }

    if (m_showAuthor) {
        m_components += MessageComponent{MessageComponentType::Author, QString(), {}};
    }

    m_components += messageContentComponents();
    endResetModel();
}

void MessageContentModel::resetContent(bool isEditing)
{
    Q_ASSERT(m_event != nullptr);

    const auto startRow = m_components[0].type == MessageComponentType::Author ? 1 : 0;
    beginRemoveRows({}, startRow, rowCount() - 1);
    m_components.remove(startRow, rowCount() - startRow);
    endRemoveRows();

    const auto newComponents = messageContentComponents(isEditing);
    if (newComponents.size() == 0) {
        return;
    }
    beginInsertRows({}, startRow, startRow + newComponents.size() - 1);
    m_components += newComponents;
    endInsertRows();
}

QList<MessageComponent> MessageContentModel::messageContentComponents(bool isEditing)
{
    QList<MessageComponent> newComponents;

    if (eventCast<const Quotient::RoomMessageEvent>(m_event)
        && eventCast<const Quotient::RoomMessageEvent>(m_event)->rawMsgtype() == QStringLiteral("m.key.verification.request")) {
        newComponents += MessageComponent{MessageComponentType::Verification, QString(), {}};
        return newComponents;
    }

    if (m_event->isRedacted()) {
        newComponents += MessageComponent{MessageComponentType::Text, QString(), {}};
        return newComponents;
    }

    if (m_replyModel != nullptr) {
        newComponents += MessageComponent{MessageComponentType::Reply, QString(), {}};
    }

    if (isEditing) {
        newComponents += MessageComponent{MessageComponentType::Edit, QString(), {}};
    } else {
        EventHandler eventHandler(m_room, m_event.get());
        newComponents.append(componentsForType(eventHandler.messageComponentType()));
    }

    if (m_room->urlPreviewEnabled()) {
        newComponents = addLinkPreviews(newComponents);
    }

    return newComponents;
}

void MessageContentModel::updateReplyModel()
{
    if (m_event == nullptr || m_replyModel != nullptr || m_isReply) {
        return;
    }

    EventHandler eventHandler(m_room, m_event.get());
    if (!eventHandler.hasReply()) {
        return;
    }

    const auto replyEvent = m_room->findInTimeline(eventHandler.getReplyId());
    if (replyEvent == m_room->historyEdge()) {
        m_replyModel = new MessageContentModel(m_room, eventHandler.getReplyId(), true);
    } else {
        m_replyModel = new MessageContentModel(m_room, replyEvent->get(), true);
    }

    connect(m_replyModel, &MessageContentModel::eventUpdated, this, [this]() {
        Q_EMIT dataChanged(index(0), index(0), {ReplyAuthorRole});
    });
}

QList<MessageComponent> MessageContentModel::componentsForType(MessageComponentType::Type type)
{
    switch (type) {
    case MessageComponentType::Text: {
        const auto event = eventCast<const Quotient::RoomMessageEvent>(m_event);
        auto body = EventHandler::rawMessageBody(*event);
        return TextHandler().textComponents(body, EventHandler::messageBodyInputFormat(*event), m_room, event, event->isReplaced());
    }
    case MessageComponentType::File: {
        QList<MessageComponent> components;
        components += MessageComponent{MessageComponentType::File, QString(), {}};
        const auto event = eventCast<const Quotient::RoomMessageEvent>(m_event);

        if (m_emptyItinerary) {
            if (!m_isReply) {
                auto fileTransferInfo = m_room->cachedFileTransferInfo(m_event.get());

#ifndef Q_OS_ANDROID
#if Quotient_VERSION_MINOR > 8
                Q_ASSERT(event->content() != nullptr && event->has<EventContent::FileContent>());
                const QMimeType mimeType = event->get<EventContent::FileContent>()->mimeType;
#else
                Q_ASSERT(event->content() != nullptr && event->hasFileContent());
                const QMimeType mimeType = event->content()->fileInfo()->mimeType;
#endif
                if (mimeType.name() == QStringLiteral("text/plain") || mimeType.parentMimeTypes().contains(QStringLiteral("text/plain"))) {
#if Quotient_VERSION_MINOR > 8
                    QString originalName = event->get<EventContent::FileContent>()->originalName;
#else
                    QString originalName = event->content()->fileInfo()->originalName;
#endif
                    if (originalName.isEmpty()) {
                        originalName = event->plainBody();
                    }
                    KSyntaxHighlighting::Repository repository;
                    KSyntaxHighlighting::Definition definitionForFile = repository.definitionForFileName(originalName);
                    if (!definitionForFile.isValid()) {
                        definitionForFile = repository.definitionForMimeType(mimeType.name());
                    }

                    QFile file(fileTransferInfo.localPath.path());
                    file.open(QIODevice::ReadOnly);
                    components += MessageComponent{MessageComponentType::Code,
                                                   QString::fromStdString(file.readAll().toStdString()),
                                                   {{QStringLiteral("class"), definitionForFile.name()}}};
                }
#endif

                if (FileType::instance().fileHasImage(fileTransferInfo.localPath)) {
                    QImageReader reader(fileTransferInfo.localPath.path());
                    components += MessageComponent{MessageComponentType::Pdf, QString(), {{QStringLiteral("size"), reader.size()}}};
                }
            }
        } else if (m_itineraryModel != nullptr) {
            components += MessageComponent{MessageComponentType::Itinerary, QString(), {}};
            if (m_itineraryModel->rowCount() > 0) {
                updateItineraryModel();
            }
        } else {
            updateItineraryModel();
        }
        auto body = EventHandler::rawMessageBody(*event);
        components += TextHandler().textComponents(body, EventHandler::messageBodyInputFormat(*event), m_room, event, event->isReplaced());
        return components;
    }
    case MessageComponentType::Image:
    case MessageComponentType::Audio:
    case MessageComponentType::Video: {
        if (!m_event->is<StickerEvent>()) {
            const auto event = eventCast<const Quotient::RoomMessageEvent>(m_event);
            QList<MessageComponent> components;
            components += MessageComponent{type, QString(), {}};
            auto body = EventHandler::rawMessageBody(*event);
            components += TextHandler().textComponents(body, EventHandler::messageBodyInputFormat(*event), m_room, event, event->isReplaced());
            return components;
        }
    }
    default:
        return {MessageComponent{type, QString(), {}}};
    }
}

MessageComponent MessageContentModel::linkPreviewComponent(const QUrl &link)
{
    const auto linkPreviewer = dynamic_cast<NeoChatConnection *>(m_room->connection())->previewerForLink(link);
    if (linkPreviewer == nullptr) {
        return {};
    }
    if (linkPreviewer->loaded()) {
        return MessageComponent{MessageComponentType::LinkPreview, QString(), {{"link"_ls, link}}};
    } else {
        connect(linkPreviewer, &LinkPreviewer::loadedChanged, this, [this, link]() {
            const auto linkPreviewer = dynamic_cast<NeoChatConnection *>(m_room->connection())->previewerForLink(link);
            if (linkPreviewer != nullptr && linkPreviewer->loaded()) {
                for (auto &component : m_components) {
                    if (component.attributes["link"_ls].toUrl() == link) {
                        // HACK: Because DelegateChooser can't switch the delegate on dataChanged it has to think there is a new delegate.
                        beginResetModel();
                        component.type = MessageComponentType::LinkPreview;
                        endResetModel();
                    }
                }
            }
        });
        return MessageComponent{MessageComponentType::LinkPreviewLoad, QString(), {{"link"_ls, link}}};
    }
}

QList<MessageComponent> MessageContentModel::addLinkPreviews(QList<MessageComponent> inputComponents)
{
    int i = 0;
    while (i < inputComponents.size()) {
        const auto component = inputComponents.at(i);
        if (component.type == MessageComponentType::Text || component.type == MessageComponentType::Quote) {
            if (LinkPreviewer::hasPreviewableLinks(component.content)) {
                const auto links = LinkPreviewer::linkPreviews(component.content);
                for (qsizetype j = 0; j < links.size(); ++j) {
                    const auto linkPreview = linkPreviewComponent(links[j]);
                    if (!m_removedLinkPreviews.contains(links[j]) && !linkPreview.isEmpty()) {
                        inputComponents.insert(i + j + 1, linkPreview);
                    }
                };
            }
        }
        i++;
    }

    return inputComponents;
}

void MessageContentModel::closeLinkPreview(int row)
{
    if (row < 0 || row > m_components.size()) {
        qWarning() << "closeLinkPreview() called with row" << row << "which does not exist. m_components.size() =" << m_components.size();
        return;
    }

    if (m_components[row].type == MessageComponentType::LinkPreview || m_components[row].type == MessageComponentType::LinkPreviewLoad) {
        beginResetModel();
        m_removedLinkPreviews += m_components[row].attributes["link"_ls].toUrl();
        m_components.remove(row);
        m_components.squeeze();
        resetContent();
    }
}

void MessageContentModel::updateItineraryModel()
{
    if (m_room == nullptr || m_event == nullptr) {
        return;
    }

    if (auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
#if Quotient_VERSION_MINOR > 8
        if (event->has<EventContent::FileContent>()) {
#else
        if (event->hasFileContent()) {
#endif
            auto filePath = m_room->cachedFileTransferInfo(event).localPath;
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
                            resetContent();
                        }
                    });
                    connect(m_itineraryModel, &ItineraryModel::loadErrorOccurred, this, [this]() {
                        m_emptyItinerary = true;
                        m_itineraryModel->deleteLater();
                        m_itineraryModel = nullptr;
                        resetContent();
                    });
                }
                m_itineraryModel->setPath(filePath.toString());
            }
        }
    }
}

#include "moc_messagecontentmodel.cpp"
