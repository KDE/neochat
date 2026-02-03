// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecontentmodel.h"

#include <QStyleHints>

#include <KLocalizedString>

#include "chatbarcache.h"
#include "contentprovider.h"
#include "neochatconnection.h"
#include "neochatdatetime.h"
#include "texthandler.h"

using namespace Quotient;

MessageContentModel::MessageContentModel(QObject *parent)
    : QAbstractListModel(parent)
{
    initializeModel();
}

MessageContentModel::MessageContentModel(NeoChatRoom *room, const QString &eventId, MessageContentModel *parent)
    : QAbstractListModel(parent)
    , m_eventId(eventId)
{
    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &MessageContentModel::updateSpoilers);

    setRoom(room);
    initializeModel();
}

void MessageContentModel::initializeModel()
{
    connect(this, &MessageContentModel::componentsUpdated, this, [this]() {
        if (!m_room) {
            return;
        }

        if (m_room->urlPreviewEnabled()) {
            forEachComponentOfType({MessageComponentType::Text, MessageComponentType::Quote}, m_linkPreviewAddFunction);
        } else {
            forEachComponentOfType({MessageComponentType::LinkPreview, MessageComponentType::LinkPreviewLoad}, m_linkPreviewRemoveFunction);
        }
        m_components.squeeze();
    });
    connect(this, &MessageContentModel::itineraryUpdated, this, [this]() {
        if (hasComponentType(MessageComponentType::File)) {
            forEachComponentOfType(MessageComponentType::File, m_fileFunction);
        }
    });
}

NeoChatRoom *MessageContentModel::room() const
{
    return m_room;
}

void MessageContentModel::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }

    if (m_room) {
        m_room->disconnect(this);
    }

    const auto oldRoom = std::exchange(m_room, room);

    if (m_room) {
        connect(m_room, &NeoChatRoom::newFileTransfer, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                forEachComponentOfType({MessageComponentType::File, MessageComponentType::Audio, MessageComponentType::Image, MessageComponentType::Video},
                                       m_fileInfoFunction);
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferProgress, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                forEachComponentOfType({MessageComponentType::File, MessageComponentType::Audio, MessageComponentType::Image, MessageComponentType::Video},
                                       m_fileInfoFunction);
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferCompleted, this, [this](const QString &eventId) {
            if (m_room != nullptr && eventId == m_eventId) {
                forEachComponentOfType({MessageComponentType::File, MessageComponentType::Audio, MessageComponentType::Image, MessageComponentType::Video},
                                       m_fileInfoFunction);
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferFailed, this, [this](const QString &eventId, const QString &errorMessage) {
            if (eventId == m_eventId) {
                forEachComponentOfType({MessageComponentType::File, MessageComponentType::Audio, MessageComponentType::Image, MessageComponentType::Video},
                                       m_fileInfoFunction);
                if (errorMessage.isEmpty()) {
                    Q_EMIT m_room->showMessage(MessageType::Error, i18nc("@info", "Failed to download file."));
                } else {
                    Q_EMIT m_room->showMessage(MessageType::Error,
                                               i18nc("@info Failed to download file: [error message]", "Failed to download file:<br />%1", errorMessage));
                }
            }
        });
        connect(m_room, &NeoChatRoom::urlPreviewEnabledChanged, this, &MessageContentModel::componentsUpdated);
    }

    Q_EMIT roomChanged(oldRoom, m_room);
}

QString MessageContentModel::eventId() const
{
    return m_eventId;
}

NeoChatDateTime MessageContentModel::dateTime() const
{
    return QDateTime::currentDateTime();
}

QString MessageContentModel::authorId() const
{
    return m_room->localMember().id();
}

NeochatRoomMember *MessageContentModel::author() const
{
    return m_room->qmlSafeMember(authorId());
}

QString MessageContentModel::threadRootId() const
{
    return {};
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

    if (!m_room) {
        qWarning() << "MessageContentModel::data called without room";
        return {};
    }

    const auto component = m_components[index.row()];

    if (role == DisplayRole) {
        return component.display;
    }
    if (role == ComponentTypeRole) {
        return component.type;
    }
    if (role == ComponentAttributesRole) {
        return component.attributes;
    }
    if (role == EventIdRole) {
        return eventId();
    }
    if (role == DateTimeRole) {
        return QVariant::fromValue(dateTime());
    }
    if (role == AuthorRole) {
        return QVariant::fromValue<NeochatRoomMember *>(author());
    }
    if (role == FileTransferInfoRole) {
        return QVariant::fromValue(m_room->cachedFileTransferInfo(m_eventId));
    }
    if (role == ItineraryModelRole) {
        return QVariant::fromValue<ItineraryModel *>(m_itineraryModel);
    }
    if (role == PollHandlerRole) {
        return QVariant::fromValue<PollHandler *>(ContentProvider::self().handlerForPoll(m_room, m_eventId));
    }
    if (role == ReplyContentModelRole) {
        return QVariant::fromValue<MessageContentModel *>(m_replyModel);
    }
    if (role == ReactionModelRole) {
        return QVariant::fromValue<ReactionModel *>(m_reactionModel);
    }
    if (role == ThreadRootRole) {
        return threadRootId();
    }
    if (role == LinkPreviewerRole) {
        if (component.type == MessageComponentType::LinkPreview) {
            return QVariant::fromValue<LinkPreviewer *>(
                dynamic_cast<NeoChatConnection *>(m_room->connection())->previewerForLink(component.attributes["link"_L1].toUrl()));
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
    if (role == EditableRole) {
        return m_editableActive;
    }
    if (role == CurrentFocusRole) {
        return index.row() == m_currentFocusComponent.row();
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
    return roleNamesStatic();
}

QHash<int, QByteArray> MessageContentModel::roleNamesStatic()
{
    QHash<int, QByteArray> roles;
    roles[MessageContentModel::DisplayRole] = "display";
    roles[MessageContentModel::ComponentTypeRole] = "componentType";
    roles[MessageContentModel::ComponentAttributesRole] = "componentAttributes";
    roles[MessageContentModel::EventIdRole] = "eventId";
    roles[MessageContentModel::DateTimeRole] = "dateTime";
    roles[MessageContentModel::AuthorRole] = "author";
    roles[MessageContentModel::FileTransferInfoRole] = "fileTransferInfo";
    roles[MessageContentModel::ItineraryModelRole] = "itineraryModel";
    roles[MessageContentModel::PollHandlerRole] = "pollHandler";
    roles[MessageContentModel::ReplyContentModelRole] = "replyContentModel";
    roles[MessageContentModel::ReactionModelRole] = "reactionModel";
    roles[MessageContentModel::ThreadRootRole] = "threadRoot";
    roles[MessageContentModel::LinkPreviewerRole] = "linkPreviewer";
    roles[MessageContentModel::ChatBarCacheRole] = "chatBarCache";
    roles[MessageContentModel::EditableRole] = "editable";
    roles[MessageContentModel::CurrentFocusRole] = "currentFocus";
    return roles;
}

bool MessageContentModel::hasComponentType(MessageComponentType::Type type) const
{
    return std::find_if(m_components.cbegin(),
                        m_components.cend(),
                        [type](const MessageComponent &component) {
                            return component.type == type;
                        })
        != m_components.cend();
}

bool MessageContentModel::hasComponentType(const QList<MessageComponentType::Type> &types) const
{
    return std::ranges::any_of(types, [this](const MessageComponentType::Type &type) {
        return hasComponentType(type);
    });
}

void MessageContentModel::forEachComponentOfType(MessageComponentType::Type type,
                                                 std::function<MessageContentModel::ComponentIt(MessageContentModel::ComponentIt)> function)
{
    auto it = m_components.begin();
    while ((it = std::find_if(it,
                              m_components.end(),
                              [type](const MessageComponent &component) {
                                  return component.type == type;
                              }))
           != m_components.end()) {
        it = function(it);
    }
}

void MessageContentModel::forEachComponentOfType(QList<MessageComponentType::Type> types,
                                                 std::function<MessageContentModel::ComponentIt(MessageContentModel::ComponentIt)> function)
{
    for (const auto &type : types) {
        forEachComponentOfType(type, function);
    }
}

std::optional<QString> MessageContentModel::getReplyEventId()
{
    return std::nullopt;
}

void MessageContentModel::updateReplyModel()
{
    const auto eventId = getReplyEventId();
    if (!eventId) {
        if (m_replyModel) {
            m_replyModel->disconnect(this);
            m_replyModel->deleteLater();
        }
        if (hasComponentType(MessageComponentType::Reply)) {
            forEachComponentOfType(MessageComponentType::Reply, [this](ComponentIt it) {
                beginRemoveRows({}, std::distance(m_components.begin(), it), std::distance(m_components.begin(), it));
                it = m_components.erase(it);
                endRemoveRows();
                return it;
            });
        }
        return;
    }

    if (m_replyModel && m_replyModel->eventId() == eventId) {
        return;
    }

    m_replyModel = new EventMessageContentModel(m_room, *eventId, true, false, this);

    if (!hasComponentType(MessageComponentType::Reply)) {
        int insertRow = 0;
        if (m_components.first().type == MessageComponentType::Author) {
            insertRow = 1;
        }
        beginInsertRows({}, insertRow, insertRow);
        m_components.insert(insertRow, MessageComponent{MessageComponentType::Reply, QString(), {}});
        endInsertRows();
    } else {
        forEachComponentOfType(MessageComponentType::Reply, [this](ComponentIt it) {
            const auto replyIndex = index(std::distance(m_components.begin(), it));
            dataChanged(replyIndex, replyIndex, {ReplyContentModelRole});
            return ++it;
        });
    }
}

MessageComponent MessageContentModel::linkPreviewComponent(const QUrl &link)
{
    const auto linkPreviewer = dynamic_cast<NeoChatConnection *>(m_room->connection())->previewerForLink(link);
    if (linkPreviewer == nullptr) {
        return {};
    }
    if (linkPreviewer->loaded()) {
        return MessageComponent{MessageComponentType::LinkPreview, QString(), {{"link"_L1, link}}};
    }
    connect(linkPreviewer, &LinkPreviewer::loadedChanged, this, [this, link]() {
        const auto linkPreviewer = dynamic_cast<NeoChatConnection *>(m_room->connection())->previewerForLink(link);
        if (linkPreviewer != nullptr && linkPreviewer->loaded()) {
            forEachComponentOfType(MessageComponentType::LinkPreviewLoad, [this, link](ComponentIt it) {
                if (it->attributes["link"_L1].toUrl() == link) {
                    it->type = MessageComponentType::LinkPreview;
                    Q_EMIT dataChanged(index(it - m_components.begin()), index(it - m_components.begin()), {ComponentTypeRole});
                }
                return ++it;
            });
        }
    });
    return MessageComponent{MessageComponentType::LinkPreviewLoad, QString(), {{"link"_L1, link}}};
}

void MessageContentModel::closeLinkPreview(int row)
{
    if (row < 0 || row >= m_components.size()) {
        qWarning() << "closeLinkPreview() called with row" << row << "which does not exist. m_components.size() =" << m_components.size();
        return;
    }

    if (m_components[row].type == MessageComponentType::LinkPreview || m_components[row].type == MessageComponentType::LinkPreviewLoad) {
        beginRemoveRows({}, row, row);
        m_removedLinkPreviews += m_components[row].attributes["link"_L1].toUrl();
        m_components.remove(row);
        m_components.squeeze();
        endRemoveRows();
    }
}

void MessageContentModel::updateSpoilers()
{
    for (auto it = m_components.begin(); it != m_components.end(); ++it) {
        updateSpoiler(index(it - m_components.begin()));
    }
}

void MessageContentModel::updateSpoiler(const QModelIndex &index)
{
    const auto row = index.row();
    if (row < 0 || row >= rowCount()) {
        qWarning() << __FUNCTION__ << "called with row" << row << "which does not exist. m_components.size() =" << m_components.size();
        return;
    }

    const auto spoilerRevealed = m_components[row].attributes.value("spoilerRevealed"_L1, false).toBool();
    m_components[row].display = TextHandler::updateSpoilerText(this, m_components[row].display, spoilerRevealed);
    Q_EMIT dataChanged(index, index, {DisplayRole});
}

void MessageContentModel::toggleSpoiler(QModelIndex index)
{
    const auto row = index.row();
    if (row < 0 || row >= rowCount()) {
        qWarning() << __FUNCTION__ << "called with row" << row << "which does not exist. m_components.size() =" << m_components.size();
        return;
    }
    if (m_components[row].type != MessageComponentType::Text) {
        return;
    }
    const auto spoilerRevealed = !m_components[row].attributes.value("spoilerRevealed"_L1, false).toBool();
    m_components[row].attributes["spoilerRevealed"_L1] = spoilerRevealed;
    Q_EMIT dataChanged(index, index, {ComponentAttributesRole});
    updateSpoiler(index);
}

#include "moc_messagecontentmodel.cpp"
