// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/events/roomevent.h>
#include <Quotient/room.h>

#include "enums/messagecomponenttype.h"
#include "eventhandler.h"
#include "itinerarymodel.h"
#include "neochatroommember.h"

struct MessageComponent {
    MessageComponentType::Type type = MessageComponentType::Other;
    QString content;
    QVariantMap attributes;

    int operator==(const MessageComponent &right) const
    {
        return type == right.type && content == right.content && attributes == right.attributes;
    }

    bool isEmpty() const
    {
        return type == MessageComponentType::Other;
    }
};

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

    /**
     * @brief Whether the author component is being shown.
     */
    Q_PROPERTY(bool showAuthor READ showAuthor WRITE setShowAuthor NOTIFY showAuthorChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        DisplayRole = Qt::DisplayRole, /**< The display text for the message. */
        ComponentTypeRole, /**< The type of component to visualise the message. */
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

        IsReplyRole, /**< Is the message a reply to another event. */
        ReplyEventIdRole, /**< The matrix ID of the message that was replied to. */
        ReplyAuthorRole, /**< The author of the event that was replied to. */
        ReplyContentModelRole, /**< The MessageContentModel for the reply event. */

        LinkPreviewerRole, /**< The link preview details. */
    };
    Q_ENUM(Roles)

    explicit MessageContentModel(NeoChatRoom *room,
                                 const Quotient::RoomEvent *event,
                                 bool isReply = false,
                                 bool isPending = false,
                                 MessageContentModel *parent = nullptr);
    MessageContentModel(NeoChatRoom *room, const QString &eventId, bool isReply = false, bool isPending = false, MessageContentModel *parent = nullptr);

    bool showAuthor() const;
    void setShowAuthor(bool showAuthor);

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

    /**
     * @brief Close the link preview at the given index.
     *
     * If the given index is not a link preview component, nothing happens.
     */
    Q_INVOKABLE void closeLinkPreview(int row);

Q_SIGNALS:
    void showAuthorChanged();
    void eventUpdated();

private:
    QPointer<NeoChatRoom> m_room;
    QString m_eventId;
    QString m_eventSenderId;
    std::unique_ptr<NeochatRoomMember> m_eventSenderObject = nullptr;
    Quotient::RoomEventPtr m_event;

    bool m_isPending;
    bool m_showAuthor = true;
    bool m_isReply;

    void initializeModel();
    void intiializeEvent(const QString &eventId);
    void intiializeEvent(const Quotient::RoomEvent *event);

    QList<MessageComponent> m_components;
    void resetModel();
    void resetContent(bool isEditing = false);
    QList<MessageComponent> messageContentComponents(bool isEditing = false);

    QPointer<MessageContentModel> m_replyModel;
    void updateReplyModel();

    ItineraryModel *m_itineraryModel = nullptr;

    QList<MessageComponent> componentsForType(MessageComponentType::Type type);
    MessageComponent linkPreviewComponent(const QUrl &link);
    QList<MessageComponent> addLinkPreviews(QList<MessageComponent> inputComponents);

    QList<QUrl> m_removedLinkPreviews;

    void updateItineraryModel();
    bool m_emptyItinerary = false;
};
