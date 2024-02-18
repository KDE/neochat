// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "eventhandler.h"
#include "linkpreviewer.h"
#include "messagecomponenttype.h"
#include "neochatroom.h"

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
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        DisplayRole = Qt::DisplayRole, /**< The display text for the message. */
        ComponentTypeRole, /**< The type of component to visualise the message. */
        EventIdRole, /**< The matrix event ID of the event. */
        AuthorRole, /**< The author of the event. */
        MediaInfoRole, /**< The media info for the event. */
        FileTransferInfoRole, /**< FileTransferInfo for any downloading files. */
        LatitudeRole, /**< Latitude for a location event. */
        LongitudeRole, /**< Longitude for a location event. */
        AssetRole, /**< Type of location event, e.g. self pin of the user location. */
        PollHandlerRole, /**< The PollHandler for the event, if any. */

        IsReplyRole, /**< Is the message a reply to another event. */
        ReplyComponentType, /**< The type of component to visualise the reply message. */
        ReplyEventIdRole, /**< The matrix ID of the message that was replied to. */
        ReplyAuthorRole, /**< The author of the event that was replied to. */
        ReplyDisplayRole, /**< The body of the message that was replied to. */
        ReplyMediaInfoRole, /**< The media info of the message that was replied to. */

        LinkPreviewerRole, /**< The link preview details. */
    };
    Q_ENUM(Roles)

    explicit MessageContentModel(const Quotient::RoomEvent *event, NeoChatRoom *room);

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

private:
    NeoChatRoom *m_room = nullptr;
    const Quotient::RoomEvent *m_event = nullptr;

    QVector<MessageComponentType::Type> m_components;
    void updateComponents();

    LinkPreviewer *m_linkPreviewer = nullptr;
};
