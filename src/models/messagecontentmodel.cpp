// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecontentmodel.h"
#include "neochatconfig.h"

#include <QImageReader>

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
    , m_event(loadEvent<RoomEvent>(event->fullJson()))
    , m_isPending(isPending)
    , m_isReply(isReply)
{
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
                updateComponents();
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
                m_event = loadEvent<RoomEvent>(serverEvent->fullJson());
                Q_EMIT eventUpdated();
                endResetModel();
            }
        }
    });
    connect(m_room, &NeoChatRoom::replacedEvent, this, [this](const Quotient::RoomEvent *newEvent) {
        if (m_room != nullptr && m_event != nullptr) {
            if (m_eventId == newEvent->id()) {
                beginResetModel();
                m_event = loadEvent<RoomEvent>(newEvent->fullJson());
                Q_EMIT eventUpdated();
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
        if (m_event != nullptr && eventId == m_eventId) {
            updateComponents();
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});

            QString mxcUrl;
            if (auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
                if (event->hasFileContent()) {
                    mxcUrl = event->content()->fileInfo()->url().toString();
                }
            } else if (auto event = eventCast<const Quotient::StickerEvent>(m_event)) {
                mxcUrl = event->image().fileInfo()->url().toString();
            }
            if (mxcUrl.isEmpty()) {
                return;
            }
            auto localPath = m_room->fileTransferInfo(m_eventId).localPath.toLocalFile();
            auto config = KSharedConfig::openStateConfig(QStringLiteral("neochatdownloads"))->group(QStringLiteral("downloads"));
            config.writePathEntry(mxcUrl.mid(6), localPath);
        }
    });
    connect(m_room, &NeoChatRoom::fileTransferFailed, this, [this](const QString &eventId) {
        if (m_event != nullptr && eventId == m_eventId) {
            updateComponents();
            Q_EMIT dataChanged(index(0), index(rowCount() - 1), {FileTransferInfoRole});
        }
    });
    connect(m_room->editCache(), &ChatBarCache::relationIdChanged, this, [this](const QString &oldEventId, const QString &newEventId) {
        if (m_event != nullptr && (oldEventId == m_eventId || newEventId == m_eventId)) {
            // HACK: Because DelegateChooser can't switch the delegate on dataChanged it has to think there is a new delegate.
            beginResetModel();
            updateComponents(newEventId == m_eventId);
            endResetModel();
        }
    });
    connect(m_room, &NeoChatRoom::urlPreviewEnabledChanged, this, [this]() {
        updateComponents();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowLinkPreviewChanged, this, [this]() {
        updateComponents();
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
    updateComponents();
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
    Q_EMIT showAuthorChanged();
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

    EventHandler eventHandler(m_room, m_event.get());
    const auto component = m_components[index.row()];

    if (role == DisplayRole) {
        if (component.type == MessageComponentType::Loading && m_isReply) {
            return i18n("Loading reply");
        }
        if (m_event == nullptr) {
            return QString();
        }
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
        return eventHandler.getTimeString(false, QLocale::ShortFormat, m_isPending, lastUpdated);
    }
    if (role == AuthorRole) {
        return QVariant::fromValue(eventHandler.getAuthor(m_isPending));
    }
    if (role == MediaInfoRole) {
        return eventHandler.getMediaInfo();
    }
    if (role == FileTransferInfoRole) {
        return QVariant::fromValue(fileInfo());
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

void MessageContentModel::updateComponents(bool isEditing)
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

    if (eventCast<const Quotient::RoomMessageEvent>(m_event)
        && eventCast<const Quotient::RoomMessageEvent>(m_event)->rawMsgtype() == QStringLiteral("m.key.verification.request")) {
        m_components += MessageComponent{MessageComponentType::Verification, QString(), {}};
        endResetModel();
        return;
    }

    if (m_event->isRedacted()) {
        m_components += MessageComponent{MessageComponentType::Text, QString(), {}};
        endResetModel();
        return;
    }

    if (m_replyModel != nullptr) {
        m_components += MessageComponent{MessageComponentType::Reply, QString(), {}};
    }

    if (isEditing) {
        m_components += MessageComponent{MessageComponentType::Edit, QString(), {}};
    } else {
        EventHandler eventHandler(m_room, m_event.get());
        m_components.append(componentsForType(eventHandler.messageComponentType()));
    }

    if (m_room->urlPreviewEnabled()) {
        addLinkPreviews();
    }

    endResetModel();
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
        auto body = EventHandler::rawMessageBody(*event);
        components += TextHandler().textComponents(body, EventHandler::messageBodyInputFormat(*event), m_room, event, event->isReplaced());
        if (m_emptyItinerary) {
            auto fileTransferInfo = fileInfo();

#ifndef Q_OS_ANDROID
            KSyntaxHighlighting::Repository repository;
            const auto definitionForFile = repository.definitionForFileName(fileTransferInfo.localPath.toString());
            if (definitionForFile.isValid() || QFileInfo(fileTransferInfo.localPath.path()).suffix() == QStringLiteral("txt")) {
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
        } else {
            updateItineraryModel();
            if (m_itineraryModel != nullptr) {
                components += MessageComponent{MessageComponentType::Itinerary, QString(), {}};
            }
        }
        return components;
    }
    case MessageComponentType::Image:
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
        connect(linkPreviewer, &LinkPreviewer::loadedChanged, [this, link]() {
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

void MessageContentModel::addLinkPreviews()
{
    int i = 0;
    while (i < m_components.size()) {
        const auto component = m_components.at(i);
        if (component.type == MessageComponentType::Text || component.type == MessageComponentType::Quote) {
            if (LinkPreviewer::hasPreviewableLinks(component.content)) {
                const auto links = LinkPreviewer::linkPreviews(component.content);
                for (qsizetype j = 0; j < links.size(); ++j) {
                    const auto linkPreview = linkPreviewComponent(links[j]);
                    if (!m_removedLinkPreviews.contains(links[j]) && !linkPreview.isEmpty()) {
                        m_components.insert(i + j + 1, linkPreview);
                    }
                };
            }
        }
        i++;
    }
}

void MessageContentModel::closeLinkPreview(int row)
{
    if (m_components[row].type == MessageComponentType::LinkPreview || m_components[row].type == MessageComponentType::LinkPreviewLoad) {
        beginResetModel();
        m_removedLinkPreviews += m_components[row].attributes["link"_ls].toUrl();
        m_components.remove(row);
        m_components.squeeze();
        updateComponents();
        endResetModel();
    }
}

void MessageContentModel::updateItineraryModel()
{
    if (m_room == nullptr || m_event == nullptr) {
        return;
    }

    if (auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
        if (event->hasFileContent()) {
            auto filePath = fileInfo().localPath;
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

FileTransferInfo MessageContentModel::fileInfo() const
{
    if (m_room == nullptr || m_event == nullptr) {
        return {};
    }

    QString mxcUrl;
    int total;
    if (auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
        if (event->hasFileContent()) {
            mxcUrl = event->content()->fileInfo()->url().toString();
            total = event->content()->fileInfo()->payloadSize;
        }
    } else if (auto event = eventCast<const Quotient::StickerEvent>(m_event)) {
        mxcUrl = event->image().fileInfo()->url().toString();
        total = event->image().fileInfo()->payloadSize;
    }
    auto config = KSharedConfig::openStateConfig(QStringLiteral("neochatdownloads"))->group(QStringLiteral("downloads"));
    if (!config.hasKey(mxcUrl.mid(6))) {
        return m_room->fileTransferInfo(m_eventId);
    }
    const auto path = config.readPathEntry(mxcUrl.mid(6), QString());
    QFileInfo info(path);
    if (!info.isFile()) {
        config.deleteEntry(mxcUrl);
        return m_room->fileTransferInfo(m_eventId);
    }
    // TODO: we could check the hash here
    return FileTransferInfo{
        .status = FileTransferInfo::Completed,
        .isUpload = false,
        .progress = total,
        .total = total,
        .localDir = QUrl(info.dir().path()),
        .localPath = QUrl::fromLocalFile(path),
    };
}

#include "moc_messagecontentmodel.cpp"
