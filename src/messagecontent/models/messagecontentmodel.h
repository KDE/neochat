// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QImageReader>
#include <QQmlEngine>

#ifndef Q_OS_ANDROID
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#endif

#include "block.h"
#include "enums/blocktype.h"
#include "filetype.h"
#include "linkpreviewer.h"
#include "models/itinerarymodel.h"
#include "models/reactionmodel.h"
#include "neochatroom.h"
#include "neochatroommember.h"

class NeoChatDateTime;

/**
 * @class MessageContentModel
 *
 * A model to visualise the content of a message.
 *
 * This is a base model designed to be extended. The inherited class needs to define
 * how the Blocks are added.
 */
class MessageContentModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The room the chat bar is for.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief The author if the message.
     */
    Q_PROPERTY(NeochatRoomMember *author READ author NOTIFY authorChanged)
    Q_PROPERTY(QString eventId READ eventId CONSTANT)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        ComponentTypeRole = Qt::UserRole, /**< The type of component to visualise the message. */
        BlockRole, /**< The Blocks::Block for the delegate. */
        EventIdRole, /**< The matrix event ID of the event. */
        DateTimeRole, /**< The timestamp for when the event was sent (as a NeoChatDateTime). */
        AuthorRole, /**< The author of the event. */
        FileTransferInfoRole, /**< FileTransferInfo for any downloading files. */
        ItineraryModelRole, /**< The itinerary model for a file. */
        PollHandlerRole, /**< The PollHandler for the event, if any. */
        ReplyContentModelRole, /**< The MessageContentModel for the reply event. */
        ReactionModelRole, /**< Reaction model for this event. */
        ThreadRootRole, /**< The thread root event ID for the event. */
        LinkPreviewerRole, /**< The link preview details. */
        ChatBarCacheRole, /**< The ChatBarCache to use. */
        EditableRole, /**< Whether the component can be edited. */
        CurrentFocusRole, /**< Whether the delegate should have focus. */
    };
    Q_ENUM(Roles)

    explicit MessageContentModel(QObject *parent = nullptr);
    explicit MessageContentModel(NeoChatRoom *room, const QString &eventId, MessageContentModel *parent = nullptr);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    static QHash<int, QByteArray> roleNamesStatic();

    /**
     * @brief The Matrix event ID of the message.
     */
    Q_INVOKABLE QString eventId() const;

    /**
     * @brief The author of the message.
     */
    Q_INVOKABLE NeochatRoomMember *author() const;

    /**
     * @brief Close the link preview at the given index.
     *
     * If the given index is not a link preview component, nothing happens.
     */
    Q_INVOKABLE void closeLinkPreview(int row);

    /**
     * @brief Toggle spoiler for the component at the given row.
     */
    Q_INVOKABLE void toggleSpoiler(QModelIndex index);

Q_SIGNALS:
    void roomChanged(NeoChatRoom *oldRoom, NeoChatRoom *newRoom);
    void authorChanged();

    /**
     * @brief Emit whenever new components are added.
     */
    void componentsUpdated();

    /**
     * @brief Emit whenever itinerary model is updated.
     */
    void itineraryUpdated();

protected:
    QPointer<NeoChatRoom> m_room;
    QString m_eventId;

    /**
     * @brief NeoChatDateTime for the message.
     *
     * The default implementation returns the current time.
     */
    virtual NeoChatDateTime dateTime() const;

    /**
     * @brief The author of the message.
     *
     * The default implementation returns the local user.
     */
    virtual QString authorId() const;

    /**
     * @brief Thread root ID for the message if in a thread.
     *
     * The default implementation returns an empty string.
     */
    virtual QString threadRootId() const;

    Blocks::BlockPtrs m_components;
    bool hasComponentType(Blocks::Type type) const;
    bool hasComponentType(const QList<Blocks::Type> &types) const;
    void forEachComponentOfType(Blocks::Type type, std::function<Blocks::BlockPtrsIt(Blocks::BlockPtrsIt)> function);
    void forEachComponentOfType(QList<Blocks::Type> types, std::function<Blocks::BlockPtrsIt(Blocks::BlockPtrsIt)> function);

    /**
     * @brief The ID for the event that the message is replying to, if any.
     *
     * The default implementation returns a std::nullopt.
     */
    virtual std::optional<QString> getReplyEventId();
    void updateReplyModel();
    QPointer<MessageContentModel> m_replyModel;
    QPointer<ReactionModel> m_reactionModel = nullptr;
    QPointer<ItineraryModel> m_itineraryModel = nullptr;
    bool m_emptyItinerary = false;

    bool m_editableActive = false;
    QPersistentModelIndex m_currentFocusComponent = {};

private:
    void initializeModel();

    std::function<Blocks::BlockPtrsIt(const Blocks::BlockPtrsIt &)> m_fileInfoFunction = [this](Blocks::BlockPtrsIt it) {
        Q_EMIT dataChanged(index(it - m_components.begin()), index(it - m_components.begin()), {MessageContentModel::FileTransferInfoRole});
        return ++it;
    };
    std::function<Blocks::BlockPtrsIt(const Blocks::BlockPtrsIt &)> m_fileFunction = [this](Blocks::BlockPtrsIt it) {
        if (m_itineraryModel && m_itineraryModel->rowCount() > 0) {
            beginInsertRows({}, std::distance(m_components.begin(), it) + 1, std::distance(m_components.begin(), it) + 1);
            it = m_components.insert(it + 1, std::make_unique<Blocks::Block>(Blocks::Itinerary, QString(), QVariantMap()));
            endInsertRows();
            return it;
        } else if (m_emptyItinerary) {
            auto fileTransferInfo = m_room->cachedFileTransferInfo(m_eventId);
#ifndef Q_OS_ANDROID
            const QMimeType mimeType = FileType::instance().mimeTypeForFile(fileTransferInfo.localPath.toString());
            if (mimeType.inherits(u"text/plain"_s)) {
                KSyntaxHighlighting::Repository repository;
                KSyntaxHighlighting::Definition definitionForFile = repository.definitionForFileName(fileTransferInfo.localPath.toString());
                if (!definitionForFile.isValid()) {
                    definitionForFile = repository.definitionForMimeType(mimeType.name());
                }

                QFile file(fileTransferInfo.localPath.path());
                auto ok = file.open(QIODevice::ReadOnly);
                if (!ok) {
                    qWarning() << "Failed to open" << fileTransferInfo.localPath.path() << file.errorString();
                }

                beginInsertRows({}, std::distance(m_components.begin(), it) + 1, std::distance(m_components.begin(), it) + 1);
                it = m_components.insert(it + 1,
                                         std::make_unique<Blocks::Block>(Blocks::Code,
                                                                         QString::fromStdString(file.readAll().toStdString()),
                                                                         QVariantMap{{u"class"_s, definitionForFile.name()}}));
                endInsertRows();
                return it;
            }
#endif

            if (FileType::instance().fileHasImage(fileTransferInfo.localPath)) {
                QImageReader reader(fileTransferInfo.localPath.path());
                beginInsertRows({}, std::distance(m_components.begin(), it) + 1, std::distance(m_components.begin(), it) + 1);
                it = m_components.insert(it + 1, std::make_unique<Blocks::Block>(Blocks::Pdf, QString(), QVariantMap{{u"size"_s, reader.size()}}));
                endInsertRows();
            }
        }
        return ++it;
    };
    std::function<Blocks::BlockPtrsIt(const Blocks::BlockPtrsIt &)> m_linkPreviewAddFunction = [this](Blocks::BlockPtrsIt it) {
        if (!m_room->urlPreviewEnabled()) {
            return it;
        }

        bool previewAdded = false;
        if (LinkPreviewer::hasPreviewableLinks(it->get()->display)) {
            const auto links = LinkPreviewer::linkPreviews(it->get()->display);
            for (qsizetype j = 0; j < links.size(); ++j) {
                auto linkPreview = linkPreviewComponent(links[j]);
                if (!m_removedLinkPreviews.contains(links[j]) && !linkPreview->isEmpty()) {
                    const auto insertIt = it + 1;
                    const auto insertRow = std::distance(m_components.begin(), insertIt);
                    beginInsertRows({}, insertRow, insertRow);
                    it = m_components.insert(insertIt, std::move(linkPreview));
                    previewAdded = true;
                    endInsertRows();
                }
            };
        }
        return previewAdded ? it : ++it;
    };
    std::function<Blocks::BlockPtrsIt(const Blocks::BlockPtrsIt &)> m_linkPreviewRemoveFunction = [this](Blocks::BlockPtrsIt it) {
        if (m_room->urlPreviewEnabled()) {
            return it;
        }
        beginRemoveRows({}, std::distance(m_components.begin(), it), std::distance(m_components.begin(), it));
        it = m_components.erase(it);
        endRemoveRows();
        return it;
    };

    QList<QUrl> m_removedLinkPreviews;
    Blocks::BlockPtr linkPreviewComponent(const QUrl &link);

    void updateSpoilers();
    void updateSpoiler(const QModelIndex &index);
};
