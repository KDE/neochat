// SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <events/roomevent.h>

#include <QAbstractListModel>

class NeoChatRoom;

namespace Quotient
{
class Connection;
class Room;
}

class NeoChatRoomType : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Defines the room list categories a room can be assigned.
     */
    enum Types {
        Invited = 1, /**< The user has been invited to the room. */
        Favorite, /**< The room is set as a favourite. */
        Direct, /**< The room is a direct chat. */
        Normal, /**< The default category for a joined room. */
        Deprioritized, /**< The room is set as low priority. */
        Space, /**< The room is a space. */
    };
    Q_ENUM(Types)
};

/**
 * @class RoomListModel
 *
 * This class defines the model for visualising the user's list of joined rooms.
 */
class RoomListModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The current connection that the model is getting its rooms from.
     */
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The total number of notifications for all the rooms.
     */
    Q_PROPERTY(int notificationCount READ notificationCount NOTIFY notificationCountChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        NameRole = Qt::UserRole + 1, /**< The name of the room. */
        DisplayNameRole, /**< The display name of the room. */
        AvatarRole, /**< The source URL for the room's avatar. */
        CanonicalAliasRole, /**< The room canonical alias. */
        TopicRole, /**< The room topic. */
        CategoryRole, /**< The room category, e.g favourite. */
        UnreadCountRole, /**< The number of unread messages in the room. */
        NotificationCountRole, /**< The number of notifications in the room. */
        HighlightCountRole, /**< The number of highlighted messages in the room. */
        LastEventRole, /**< Text for the last event in the room. */
        LastActiveTimeRole, /**< The timestamp of the last event sent in the room. */
        JoinStateRole, /**< The local user's join state in the room. */
        CurrentRoomRole, /**< The room object for the room. */
        CategoryVisibleRole, /**< If the room's category is visible. */
        SubtitleTextRole, /**< The text to show as the room subtitle. */
        AvatarImageRole, /**< The room avatar as an image. */
        IdRole, /**< The room matrix ID. */
        IsSpaceRole, /**< Whether the room is a space. */
    };
    Q_ENUM(EventRoles)

    RoomListModel(QObject *parent = nullptr);
    ~RoomListModel() override;

    [[nodiscard]] Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *connection);

    [[nodiscard]] int notificationCount() const;

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
     * @brief Return a string to represent the given room category.
     */
    Q_INVOKABLE [[nodiscard]] static QString categoryName(int category);

    /**
     * @brief Return a string with the name of the given room category icon.
     */
    Q_INVOKABLE [[nodiscard]] static QString categoryIconName(int category);

    /**
     * @brief Set whether a given category should be visible or not.
     *
     * @param category the NeoChatRoomType::Types value for the category (it's an
     *                 int due to the pain of Q_INVOKABLES and cpp enums).
     * @param visible true if the category should be visible, false if not.
     */
    Q_INVOKABLE void setCategoryVisible(int category, bool visible);

    /**
     * @brief Return whether a room category is set to be visible.
     */
    Q_INVOKABLE [[nodiscard]] bool categoryVisible(int category) const;

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
    void refresh(NeoChatRoom *room, const QVector<int> &roles = {});
    void refreshNotificationCount();

private:
    Quotient::Connection *m_connection = nullptr;
    QList<NeoChatRoom *> m_rooms;

    QMap<int, bool> m_categoryVisibility;

    int m_notificationCount = 0;
    QString m_activeSpaceId = "";

    void connectRoomSignals(NeoChatRoom *room);
#ifndef QUOTIENT_07
    void handleNotifications();
#endif

Q_SIGNALS:
    void connectionChanged();
    void notificationCountChanged();

    void roomAdded(NeoChatRoom *_t1);
};
