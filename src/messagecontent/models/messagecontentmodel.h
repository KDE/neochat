// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/events/roomevent.h>

#include "enums/messagecomponenttype.h"
#include "messagecomponent.h"
#include "models/itinerarymodel.h"
#include "models/reactionmodel.h"
#include "neochatroommember.h"

class ThreadModel;

/**
 * @class MessageContentModel
 *
 * A model to visualise the components of a single RoomMessageEvent.
 */
class MessageContentModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    enum MessageState {
        Unknown, /**< The message state is unknown. */
        Pending, /**< The message is a new pending message which the server has not yet acknowledged. */
        Available, /**< The message is available and acknowledged by the server. */
        UnAvailable, /**< The message can't be retrieved either because it doesn't exist or is blocked. */
    };
    Q_ENUM(MessageState)

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
        MediaInfoRole, /**< The media info for the event. */
        FileTransferInfoRole, /**< FileTransferInfo for any downloading files. */
        ItineraryModelRole, /**< The itinerary model for a file. */
        LatitudeRole, /**< Latitude for a location event. */
        LongitudeRole, /**< Longitude for a location event. */
        AssetRole, /**< Type of location event, e.g. self pin of the user location. */
        PollHandlerRole, /**< The PollHandler for the event, if any. */

        ReplyEventIdRole, /**< The matrix ID of the message that was replied to. */
        ReplyAuthorRole, /**< The author of the event that was replied to. */
        ReplyContentModelRole, /**< The MessageContentModel for the reply event. */

        ReactionModelRole, /**< Reaction model for this event. */

        ThreadRootRole, /**< The thread root event ID for the event. */

        LinkPreviewerRole, /**< The link preview details. */
        ChatBarCacheRole, /**< The ChatBarCache to use. */
    };
    Q_ENUM(Roles)

    explicit MessageContentModel(NeoChatRoom *room,
                                 const QString &eventId,
                                 bool isReply = false,
                                 bool isPending = false,
                                 MessageContentModel *parent = nullptr);

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
     * @brief Close the link preview at the given index.
     *
     * If the given index is not a link preview component, nothing happens.
     */
    Q_INVOKABLE void closeLinkPreview(int row);

    /**
     * @brief Returns the thread model for the given thread root event ID.
     *
     * A model is created is one doesn't exist. Will return nullptr if threadRootId
     * is empty.
     */
    Q_INVOKABLE ThreadModel *modelForThread(const QString &threadRootId);

    static void setThreadsEnabled(bool enableThreads);

Q_SIGNALS:
    void showAuthorChanged();
    void eventUpdated();

    void threadsEnabledChanged();

private:
    QPointer<NeoChatRoom> m_room;
    QString m_eventId;
    QString senderId() const;
    NeochatRoomMember *senderObject() const;

    MessageState m_currentState = Unknown;
    bool m_isReply;

    void initializeModel();
    void initializeEvent();
    void getEvent();
    void updateFileInfo();

    QList<MessageComponent> m_components;
    void resetModel();
    void resetContent(bool isEditing = false, bool isThreading = false);
    QList<MessageComponent> messageContentComponents(bool isEditing = false, bool isThreading = false);

    QPointer<MessageContentModel> m_replyModel;
    void updateReplyModel();

    ReactionModel *m_reactionModel = nullptr;
    ItineraryModel *m_itineraryModel = nullptr;

    QList<MessageComponent> componentsForType(MessageComponentType::Type type);
    MessageComponent linkPreviewComponent(const QUrl &link);
    QList<MessageComponent> addLinkPreviews(QList<MessageComponent> inputComponents);

    QList<QUrl> m_removedLinkPreviews;

    void updateItineraryModel();
    bool m_emptyItinerary = false;

    void updateReactionModel();

    static bool m_threadsEnabled;
};
