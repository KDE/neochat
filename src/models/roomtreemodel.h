
// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractItemModel>
#include <QPointer>

#include "roomtreeitem.h"

namespace Integral
{
class Connection;
class Room;
}

class RoomTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Integral::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

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
        JoinStateRole, /**< The local user's join state in the room. */
        CurrentRoomRole, /**< The room object for the room. */
        SubtitleTextRole, /**< The text to show as the room subtitle. */
        AvatarImageRole, /**< The room avatar as an image. */
        RoomIdRole, /**< The room matrix ID. */
        IsSpaceRole, /**< Whether the room is a space. */
        IsChildSpaceRole, /**< Whether this space is a child of a different space. */
        ReplacementIdRole, /**< The room id of the room replacing this one, if any. */
        IsDirectChat, /**< Whether this room is a direct chat. */
        DelegateTypeRole,
        IconRole,
        RoomTypeRole, /**< The room's type. */
    };
    Q_ENUM(EventRoles)
    explicit RoomTreeModel(QObject *parent = nullptr);
    ~RoomTreeModel();

    void setConnection(Integral::Connection *connection);
    Integral::Connection *connection() const;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex indexForRoom(rust::Box<sdk::RoomListRoom> room) const;
    std::optional<rust::Box<sdk::RoomListRoom>> roomForIndex(QModelIndex index) const;

Q_SIGNALS:
    void connectionChanged();

private:
    class Private;
    std::unique_ptr<Private> d;

    RoomTreeItem *getItem(const QModelIndex &index) const;

    void resetModel();

    // void connectRoomSignals(NeoChatRoom *room);

    // void refreshRoomRoles(NeoChatRoom *room, const QList<int> &roles = {});
};
