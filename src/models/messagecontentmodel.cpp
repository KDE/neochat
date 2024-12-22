// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecontentmodel.h"
#include "eventhandler.h"
#include "neochatconfig.h"

#include <QImageReader>

#include <Quotient/events/eventcontent.h>
#include <Quotient/events/redactionevent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/qt_connection_util.h>
#if Quotient_VERSION_MINOR > 9
#include <Quotient/thread.h>
#endif

#include <KLocalizedString>
#include <Kirigami/Platform/PlatformTheme>

#ifndef Q_OS_ANDROID
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#endif

#include "chatbarcache.h"
#include "filetype.h"
#include "linkpreviewer.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "texthandler.h"

using namespace Quotient;

MessageContentModel::MessageContentModel(NeoChatRoom *room, const QString &eventId, bool isReply, bool isPending, MessageContentModel *parent)
    : QAbstractListModel(parent)
    , m_room(room)
    , m_eventId(eventId)
    , m_currentState(isPending ? Pending : Unknown)
    , m_isReply(isReply)
{
    initializeModel();
}

void MessageContentModel::initializeModel()
{
    Q_ASSERT(m_room != nullptr);
    Q_ASSERT(!m_eventId.isEmpty());

    connect(m_room, &NeoChatRoom::pendingEventAdded, this, [this]() {
        if (m_room != nullptr && m_currentState == Unknown) {
            initializeEvent();
            updateReplyModel();
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
            updateReplyModel();
            resetModel();
        }
    });
    connect(m_room, &NeoChatRoom::addedMessages, this, [this](int fromIndex, int toIndex) {
        if (m_room != nullptr) {
            for (int i = fromIndex; i <= toIndex; i++) {
                if (m_room->findInTimeline(i)->event()->id() == m_eventId) {
                    initializeEvent();
                    updateReplyModel();
                    resetModel();
                }
            }
        }
    });
    connect(m_room, &NeoChatRoom::replacedEvent, this, [this](const Quotient::RoomEvent *newEvent) {
        if (m_room != nullptr) {
            if (m_eventId == newEvent->id()) {
                beginResetModel();
                initializeEvent();
                resetContent();
                endResetModel();
            }
        }
    });
    connect(m_room, &NeoChatRoom::newFileTransfer, this, [this](const QString &eventId) {
        if (eventId == m_eventId) {
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
        }
    });
    connect(m_room, &NeoChatRoom::fileTransferProgress, this, [this](const QString &eventId) {
        if (eventId == m_eventId) {
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
        }
    });
    connect(m_room, &NeoChatRoom::fileTransferCompleted, this, [this](const QString &eventId) {
        if (m_room != nullptr && eventId == m_eventId) {
            resetContent();
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
        }
    });
    connect(m_room, &NeoChatRoom::fileTransferFailed, this, [this](const QString &eventId) {
        if (eventId == m_eventId) {
            resetContent();
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
        }
    });
    connect(m_room->editCache(), &ChatBarCache::relationIdChanged, this, [this](const QString &oldEventId, const QString &newEventId) {
        if (oldEventId == m_eventId || newEventId == m_eventId) {
            // HACK: Because DelegateChooser can't switch the delegate on dataChanged it has to think there is a new delegate.
            beginResetModel();
            resetContent(newEventId == m_eventId);
            endResetModel();
        }
    });
    connect(m_room->threadCache(), &ChatBarCache::threadIdChanged, this, [this](const QString &oldThreadId, const QString &newThreadId) {
        if (oldThreadId == m_eventId || newThreadId == m_eventId) {
            beginResetModel();
            resetContent(false, newThreadId == m_eventId);
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
        if (m_room != nullptr) {
            if (m_eventSenderId.isEmpty() || m_eventSenderId == member.id()) {
                Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole});
            }
        }
    });
    connect(m_room, &Room::memberAvatarUpdated, this, [this](RoomMember member) {
        if (m_room != nullptr) {
            if (m_eventSenderId.isEmpty() || m_eventSenderId == member.id()) {
                Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole});
            }
        }
    });

    connect(NeoChatConfig::self(), &NeoChatConfig::ThreadsChanged, this, [this]() {
        updateReplyModel();
        resetModel();
    });

    initializeEvent();
    if (m_currentState == Available || m_currentState == Pending) {
        updateReplyModel();
    }
    resetModel();
}

void MessageContentModel::initializeEvent()
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

    if (m_eventSenderObject == nullptr) {
        auto senderId = eventResult.first->senderId();
        // A pending event might not have a sender ID set yet but in that case it must
        // be the local member.
        if (senderId.isEmpty()) {
            senderId = m_room->localMember().id();
        }
        m_eventSenderObject = std::unique_ptr<NeochatRoomMember>(new NeochatRoomMember(m_room, senderId));
    }
    Q_EMIT eventUpdated();
}

void MessageContentModel::getEvent()
{
    Quotient::connectUntil(m_room.get(), &NeoChatRoom::extraEventLoaded, this, [this](const QString &eventId) {
        if (m_room != nullptr) {
            if (eventId == m_eventId) {
                initializeEvent();
                updateReplyModel();
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

    if (m_room->connection()->isIgnored(m_eventSenderId)) {
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

    const auto component = m_components[index.row()];

    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr) {
        if (role == DisplayRole) {
            if (m_isReply) {
                return i18n("Loading reply");
            } else {
                return i18n("Loading");
            }
        }
        if (role == ComponentTypeRole) {
            return component.type;
        }
        return {};
    }

    if (role == DisplayRole) {
        if (m_currentState == UnAvailable || m_room->connection()->isIgnored(m_eventSenderId)) {
            Kirigami::Platform::PlatformTheme *theme =
                static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));

            QString disabledTextColor;
            if (theme != nullptr) {
                disabledTextColor = theme->disabledTextColor().name();
            } else {
                disabledTextColor = QStringLiteral("#000000");
            }
            return QString(QStringLiteral("<span style=\"color:%1\">").arg(disabledTextColor)
                           + i18nc("@info", "This message was either not found, you do not have permission to view it, or it was sent by an ignored user")
                           + QStringLiteral("</span>"));
        }
        if (component.type == MessageComponentType::Loading) {
            if (m_isReply) {
                return i18n("Loading reply");
            } else {
                return i18n("Loading");
            }
        }
        if (!component.content.isEmpty()) {
            return component.content;
        }
        return EventHandler::richBody(m_room, event.first);
    }
    if (role == ComponentTypeRole) {
        return component.type;
    }
    if (role == ComponentAttributesRole) {
        return component.attributes;
    }
    if (role == EventIdRole) {
        return event.first->displayId();
    }
    if (role == TimeRole) {
        const auto pendingIt = std::find_if(m_room->pendingEvents().cbegin(), m_room->pendingEvents().cend(), [event](const PendingEventItem &pendingEvent) {
            return event.first->transactionId() == pendingEvent->transactionId();
        });

        auto lastUpdated = pendingIt == m_room->pendingEvents().cend() ? QDateTime() : pendingIt->lastUpdated();
        return EventHandler::time(event.first, m_currentState == Pending, lastUpdated);
    }
    if (role == TimeStringRole) {
        const auto pendingIt = std::find_if(m_room->pendingEvents().cbegin(), m_room->pendingEvents().cend(), [event](const PendingEventItem &pendingEvent) {
            return event.first->transactionId() == pendingEvent->transactionId();
        });

        auto lastUpdated = pendingIt == m_room->pendingEvents().cend() ? QDateTime() : pendingIt->lastUpdated();
        return EventHandler::timeString(event.first, QStringLiteral("hh:mm"), m_currentState == Pending, lastUpdated);
    }
    if (role == AuthorRole) {
        return QVariant::fromValue<NeochatRoomMember *>(m_eventSenderObject.get());
    }
    if (role == MediaInfoRole) {
        return EventHandler::mediaInfo(m_room, event.first);
    }
    if (role == FileTransferInfoRole) {
        return QVariant::fromValue(m_room->cachedFileTransferInfo(event.first));
    }
    if (role == ItineraryModelRole) {
        return QVariant::fromValue<ItineraryModel *>(m_itineraryModel);
    }
    if (role == LatitudeRole) {
        return EventHandler::latitude(event.first);
    }
    if (role == LongitudeRole) {
        return EventHandler::longitude(event.first);
    }
    if (role == AssetRole) {
        return EventHandler::locationAssetType(event.first);
    }
    if (role == PollHandlerRole) {
        return QVariant::fromValue<PollHandler *>(m_room->poll(m_eventId));
    }
    if (role == ReplyEventIdRole) {
        if (const auto roomMessageEvent = eventCast<const RoomMessageEvent>(event.first)) {
            return roomMessageEvent->replyEventId();
        }
    }
    if (role == ReplyAuthorRole) {
        return QVariant::fromValue(EventHandler::replyAuthor(m_room, event.first));
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
    if (role == ChatBarCacheRole) {
        if (m_room->threadCache()->threadId() == m_eventId) {
            return QVariant::fromValue<ChatBarCache *>(m_room->threadCache());
        }
        return QVariant::fromValue<ChatBarCache *>(m_room->editCache());
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
    roles[ReplyEventIdRole] = "replyEventId";
    roles[ReplyAuthorRole] = "replyAuthor";
    roles[ReplyContentModelRole] = "replyContentModel";
    roles[LinkPreviewerRole] = "linkPreviewer";
    roles[ChatBarCacheRole] = "chatBarCache";
    return roles;
}

void MessageContentModel::resetModel()
{
    beginResetModel();
    m_components.clear();

    if (m_room->connection()->isIgnored(m_eventSenderId) || m_currentState == UnAvailable) {
        m_components += MessageComponent{MessageComponentType::Text, QString(), {}};
        endResetModel();
        return;
    }

    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr) {
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

void MessageContentModel::resetContent(bool isEditing, bool isThreading)
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
}

QList<MessageComponent> MessageContentModel::messageContentComponents(bool isEditing, bool isThreading)
{
    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr) {
        return {};
    }

    QList<MessageComponent> newComponents;

    const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event.first);
    if (roomMessageEvent && roomMessageEvent->rawMsgtype() == QStringLiteral("m.key.verification.request")) {
        newComponents += MessageComponent{MessageComponentType::Verification, QString(), {}};
        return newComponents;
    }

    if (event.first->isRedacted()) {
        newComponents += MessageComponent{MessageComponentType::Text, QString(), {}};
        return newComponents;
    }

    if (m_replyModel != nullptr) {
        newComponents += MessageComponent{MessageComponentType::Reply, QString(), {}};
    }

    if (isEditing) {
        newComponents += MessageComponent{MessageComponentType::ChatBar, QString(), {}};
    } else {
        newComponents.append(componentsForType(MessageComponentType::typeForEvent(*event.first)));
    }

    if (m_room->urlPreviewEnabled()) {
        newComponents = addLinkPreviews(newComponents);
    }

    // If the event is already threaded the ThreadModel will handle displaying a chat bar.
#if Quotient_VERSION_MINOR > 9
    if (isThreading && roomMessageEvent && !(roomMessageEvent->isThreaded() || m_room->threads().contains(roomMessageEvent->id()))) {
#else
    if (isThreading && roomMessageEvent && roomMessageEvent->isThreaded()) {
#endif
        newComponents += MessageComponent{MessageComponentType::ChatBar, QString(), {}};
    }

    return newComponents;
}

void MessageContentModel::updateReplyModel()
{
    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr || m_isReply) {
        return;
    }

    const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event.first);
    if (roomMessageEvent == nullptr) {
        return;
    }
    if (!roomMessageEvent->isReply() || (roomMessageEvent->isThreaded() && NeoChatConfig::self()->threads())) {
        if (m_replyModel) {
            delete m_replyModel;
        }
        return;
    }

    if (m_replyModel != nullptr) {
        return;
    }

    m_replyModel = new MessageContentModel(m_room, roomMessageEvent->replyEventId(), true, false, this);

    connect(m_replyModel, &MessageContentModel::eventUpdated, this, [this]() {
        Q_EMIT dataChanged(index(0), index(0), {ReplyAuthorRole});
    });
}

QList<MessageComponent> MessageContentModel::componentsForType(MessageComponentType::Type type)
{
    const auto event = m_room->getEvent(m_eventId);
    if (event.first == nullptr) {
        return {};
    }

    switch (type) {
    case MessageComponentType::Text: {
        const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event.first);
        auto body = EventHandler::rawMessageBody(*roomMessageEvent);
        return TextHandler().textComponents(body,
                                            EventHandler::messageBodyInputFormat(*roomMessageEvent),
                                            m_room,
                                            roomMessageEvent,
                                            roomMessageEvent->isReplaced());
    }
    case MessageComponentType::File: {
        QList<MessageComponent> components;
        components += MessageComponent{MessageComponentType::File, QString(), {}};
        const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event.first);

        if (m_emptyItinerary) {
            if (!m_isReply) {
                auto fileTransferInfo = m_room->cachedFileTransferInfo(event.first);

#ifndef Q_OS_ANDROID
                Q_ASSERT(roomMessageEvent->content() != nullptr && roomMessageEvent->has<EventContent::FileContent>());
                const QMimeType mimeType = roomMessageEvent->get<EventContent::FileContent>()->mimeType;
                if (mimeType.name() == QStringLiteral("text/plain") || mimeType.parentMimeTypes().contains(QStringLiteral("text/plain"))) {
                    QString originalName = roomMessageEvent->get<EventContent::FileContent>()->originalName;
                    if (originalName.isEmpty()) {
                        originalName = roomMessageEvent->plainBody();
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
        auto body = EventHandler::rawMessageBody(*roomMessageEvent);
        components += TextHandler().textComponents(body,
                                                   EventHandler::messageBodyInputFormat(*roomMessageEvent),
                                                   m_room,
                                                   roomMessageEvent,
                                                   roomMessageEvent->isReplaced());
        return components;
    }
    case MessageComponentType::Image:
    case MessageComponentType::Audio:
    case MessageComponentType::Video: {
        if (!event.first->is<StickerEvent>()) {
            const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event.first);
            const auto fileContent = roomMessageEvent->get<EventContent::FileContentBase>();
            if (fileContent != nullptr) {
                const auto fileInfo = fileContent->commonInfo();
                const auto body = EventHandler::rawMessageBody(*roomMessageEvent);
                // Do not attach the description to the image, if it's the same as the original filename.
                if (fileInfo.originalName != body) {
                    QList<MessageComponent> components;
                    components += MessageComponent{type, QString(), {}};
                    components += TextHandler().textComponents(body,
                                                               EventHandler::messageBodyInputFormat(*roomMessageEvent),
                                                               m_room,
                                                               roomMessageEvent,
                                                               roomMessageEvent->isReplaced());
                    return components;
                }
            }
        }
    }
        [[fallthrough]];
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
    if (row < 0 || row >= m_components.size()) {
        qWarning() << "closeLinkPreview() called with row" << row << "which does not exist. m_components.size() =" << m_components.size();
        return;
    }

    if (m_components[row].type == MessageComponentType::LinkPreview || m_components[row].type == MessageComponentType::LinkPreviewLoad) {
        beginResetModel();
        m_removedLinkPreviews += m_components[row].attributes["link"_ls].toUrl();
        m_components.remove(row);
        m_components.squeeze();
        endResetModel();
        resetContent();
    }
}

void MessageContentModel::updateItineraryModel()
{
    const auto event = m_room->getEvent(m_eventId);
    if (m_room == nullptr || event.first == nullptr) {
        return;
    }

    if (auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event.first)) {
        if (roomMessageEvent->has<EventContent::FileContent>()) {
            auto filePath = m_room->cachedFileTransferInfo(event.first).localPath;
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
