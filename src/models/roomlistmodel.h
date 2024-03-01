// SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "enums/neochatroomtype.h"

class NeoChatRoom;

namespace Quotient
{
class Connection;
class Room;
}

/**
 * @class RoomListModel
 *
 * This class defines the model for visualising the user's list of joined rooms.
 */
class RoomListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current connection that the model is getting its rooms from.
     */
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        DisplayNameRole = Qt::DisplayRole, /**< The display name of the room. */
        AvatarRole, /**< The source URL for the room's avatar. */
        CanonicalAliasRole, /**< The room canonical alias. */
        TopicRole, /**< The room topic. */
        CategoryRole, /**< The room category, e.g favourite. */
        ContextNotificationCountRole, /**< The context aware notification count for the room. */
        HasHighlightNotificationsRole, /**< Whether there are any highlight notifications. */
        LastActiveTimeRole, /**< The timestamp of the last event sent in the room. */
        JoinStateRole, /**< The local user's join state in the room. */
        CurrentRoomRole, /**< The room object for the room. */
        SubtitleTextRole, /**< The text to show as the room subtitle. */
        AvatarImageRole, /**< The room avatar as an image. */
        RoomIdRole, /**< The room matrix ID. */
        IsSpaceRole, /**< Whether the room is a space. */
        IsChildSpaceRole, /**< Whether this space is a child of a different space. */
        ReplacementIdRole, /**< The room id of the room replacing this one, if any. */
        IsDirectChat, /**< Whether this room is a direct chat. */
    };
    Q_ENUM(EventRoles)

    explicit RoomListModel(QObject *parent = nullptr);
    ~RoomListModel() override;

    [[nodiscard]] Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *connection);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    Q_INVOKABLE [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Return the room at the given row.
     */
    Q_INVOKABLE [[nodiscard]] NeoChatRoom *roomAt(int row) const;

    /**
     * @brief Return the model row for the given room.
     */
    Q_INVOKABLE [[nodiscard]] int rowForRoom(NeoChatRoom *room) const;

    /**
     * @brief Return a room for the given room alias or room matrix ID.
     *
     * The room must be in the model.
     */
    Q_INVOKABLE NeoChatRoom *roomByAliasOrId(const QString &aliasOrId);

private Q_SLOTS:
    void doResetModel();
    void doAddRoom(Quotient::Room *room);
    void updateRoom(Quotient::Room *room, Quotient::Room *prev);
    void deleteRoom(Quotient::Room *room);
    void refresh(NeoChatRoom *room, const QList<int> &roles = {});

private:
    Quotient::Connection *m_connection = nullptr;
    QList<NeoChatRoom *> m_rooms;

    QString m_activeSpaceId;

    void connectRoomSignals(NeoChatRoom *room);

Q_SIGNALS:
    void connectionChanged();
    void roomAdded(NeoChatRoom *_t1);
};
