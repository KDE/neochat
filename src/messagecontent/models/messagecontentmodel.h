// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QImageReader>

#ifndef Q_OS_ANDROID
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#endif

#include "enums/messagecomponenttype.h"
#include "filetype.h"
#include "linkpreviewer.h"
#include "messagecomponent.h"
#include "models/itinerarymodel.h"
#include "models/reactionmodel.h"
#include "neochatroom.h"
#include "neochatroommember.h"

/**
 * @class MessageContentModel
 *
 * A model to visualise the content of a message.
 *
 * This is a base model designed to be extended. The inherited class needs to define
 * how the MessageComponents are added.
 */
class MessageContentModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    Q_PROPERTY(NeochatRoomMember *author READ author NOTIFY authorChanged)
    Q_PROPERTY(QString eventId READ eventId CONSTANT)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        DisplayRole = Qt::DisplayRole, /**< The display text for the message. */
        ComponentTypeRole = Qt::UserRole, /**< The type of component to visualise the message. */
        ComponentAttributesRole, /**< The attributes of the component. */
        EventIdRole, /**< The matrix event ID of the event. */
        TimeRole, /**< The timestamp for when the event was sent (as a QDateTime). */
        TimeStringRole, /**< The timestamp for when the event was sent as a string (in QLocale::ShortFormat). */
        AuthorRole, /**< The author of the event. */
        FileTransferInfoRole, /**< FileTransferInfo for any downloading files. */
        ItineraryModelRole, /**< The itinerary model for a file. */
        PollHandlerRole, /**< The PollHandler for the event, if any. */
        ReplyContentModelRole, /**< The MessageContentModel for the reply event. */
        ReactionModelRole, /**< Reaction model for this event. */
        ThreadRootRole, /**< The thread root event ID for the event. */
        LinkPreviewerRole, /**< The link preview details. */
        ChatBarCacheRole, /**< The ChatBarCache to use. */
    };
    Q_ENUM(Roles)

    explicit MessageContentModel(NeoChatRoom *room, MessageContentModel *parent = nullptr, const QString &eventId = {});

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
     * @brief QDateTime for the message.
     *
     * The default implementation returns the current time.
     */
    virtual QDateTime time() const;

    /**
     * @brief Time for the message as a string in the from "hh:mm".
     *
     * The default implementation returns the current time.
     */
    virtual QString timeString() const;

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

    using ComponentIt = QList<MessageComponent>::iterator;

    QList<MessageComponent> m_components;
    bool hasComponentType(MessageComponentType::Type type);
    void forEachComponentOfType(MessageComponentType::Type type, std::function<ComponentIt(ComponentIt)> function);
    void forEachComponentOfType(QList<MessageComponentType::Type> types, std::function<ComponentIt(ComponentIt)> function);

    QPointer<MessageContentModel> m_replyModel;
    QPointer<ReactionModel> m_reactionModel = nullptr;
    QPointer<ItineraryModel> m_itineraryModel = nullptr;
    bool m_emptyItinerary = false;

private:
    void initializeModel();

    std::function<ComponentIt(const ComponentIt &)> m_fileInfoFunction = [this](ComponentIt it) {
        Q_EMIT dataChanged(index(it - m_components.begin()), index(it - m_components.begin()), {MessageContentModel::FileTransferInfoRole});
        return ++it;
    };
    std::function<ComponentIt(const ComponentIt &)> m_fileFunction = [this](ComponentIt it) {
        if (m_itineraryModel && m_itineraryModel->rowCount() > 0) {
            beginInsertRows({}, std::distance(m_components.begin(), it) + 1, std::distance(m_components.begin(), it) + 1);
            it = m_components.insert(it + 1, MessageComponent{MessageComponentType::Itinerary, QString(), {}});
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
                it = m_components.insert(it + 1, MessageComponent{MessageComponentType::Code,  QString::fromStdString(file.readAll().toStdString()), {{u"class"_s, definitionForFile.name()}}});
                endInsertRows();
                return it;
            }
#endif

            if (FileType::instance().fileHasImage(fileTransferInfo.localPath)) {
                QImageReader reader(fileTransferInfo.localPath.path());
                beginInsertRows({}, std::distance(m_components.begin(), it) + 1, std::distance(m_components.begin(), it) + 1);
                it = m_components.insert(it + 1, MessageComponent{MessageComponentType::Pdf, QString(), {{u"size"_s, reader.size()}}});
                endInsertRows();
            }
        }
        return ++it;
    };
    std::function<ComponentIt(const ComponentIt &)> m_linkPreviewAddFunction = [this](ComponentIt it) {
        if (!m_room->urlPreviewEnabled()) {
            return it;
        }

        bool previewAdded = false;
        if (LinkPreviewer::hasPreviewableLinks(it->display)) {
            const auto links = LinkPreviewer::linkPreviews(it->display);
            for (qsizetype j = 0; j < links.size(); ++j) {
                const auto linkPreview = linkPreviewComponent(links[j]);
                if (!m_removedLinkPreviews.contains(links[j]) && !linkPreview.isEmpty()) {
                    const auto insertRow = std::distance(m_components.begin(), it) + 1;
                    beginInsertRows({}, insertRow, insertRow);
                    it = m_components.insert(insertRow, linkPreview);
                    previewAdded = true;
                    endInsertRows();
                }
            };
        }
        return previewAdded ? it : ++it;
    };
    std::function<ComponentIt(const ComponentIt &)> m_linkPreviewRemoveFunction = [this](ComponentIt it) {
        if (m_room->urlPreviewEnabled()) {
            return it;
        }
        beginRemoveRows({}, std::distance(m_components.begin(), it), std::distance(m_components.begin(), it));
        it = m_components.erase(it);
        endRemoveRows();
        return it;
    };

    QList<QUrl> m_removedLinkPreviews;
    MessageComponent linkPreviewComponent(const QUrl &link);

    void updateSpoilers();
    void updateSpoiler(const QModelIndex &index);
};
