// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractItemModel>
#include <QPointer>

#include "enums/neochatroomtype.h"

namespace Quotient
{
class Room;
}

class NeoChatConnection;
class NeoChatRoom;

class TreeItem
{
public:
    using TreeData = std::variant<NeoChatRoom *, NeoChatRoomType::Types>;

    explicit TreeItem(TreeData data, TreeItem *parentItem = nullptr);

    TreeItem *child(int row);
    int childCount() const;
    QVariant data(int role) const;
    void appendChild(std::unique_ptr<TreeItem> &&child);
    bool insertChildren(int position, int count, TreeData treeData);
    TreeItem *parentItem() const;
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int row() const;
    TreeData treeData() const;

private:
    std::vector<std::unique_ptr<TreeItem>> m_childItems;
    TreeData m_treeData;
    TreeItem *m_parentItem;
};

class RoomTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

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
        DelegateTypeRole,
        IconRole,
        AttentionRole, /**< Whether there are any notifications. */
        FavouriteRole, /**< Whether the room is favourited. */
    };
    Q_ENUM(EventRoles)
    explicit RoomTreeModel(QObject *parent = nullptr);

    void setConnection(NeoChatConnection *connection);
    NeoChatConnection *connection() const;

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

    TreeItem *getItem(const QModelIndex &index) const;

    Q_INVOKABLE QModelIndex indexForRoom(NeoChatRoom *room) const;

Q_SIGNALS:
    void connectionChanged();

private:
    QPointer<NeoChatConnection> m_connection = nullptr;

    void initializeCategories();
    void connectRoomSignals(NeoChatRoom *room);

    void newRoom(Quotient::Room *room);
    void leftRoom(Quotient::Room *room);
    void moveRoom(Quotient::Room *room);

    void refreshRoomRoles(NeoChatRoom *room, const QList<int> &roles = {});

    std::unique_ptr<TreeItem> m_rootItem;
};
