/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "roomlistmodel.h"

#include "neochatconfig.h"
#include "user.h"
#include "utils.h"

#include "events/roomevent.h"

#include <QBrush>
#include <QColor>
#include <QDebug>
#ifndef Q_OS_ANDROID
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#endif
#include <QStandardPaths>

#include <KLocalizedString>
#include <utility>

#ifndef Q_OS_ANDROID
bool useUnityCounter() {
    static const auto Result = QDBusInterface(
        "com.canonical.Unity",
        "/").isValid();

    return Result;
}
#endif

RoomListModel::RoomListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const auto collapsedSections = NeoChatConfig::collapsedSections();
    for (auto collapsedSection : collapsedSections) {
        m_categoryVisibility[collapsedSection] = false;
    }

#ifndef Q_OS_ANDROID
    connect(this, &RoomListModel::notificationCountChanged, this, [this]() {
        if (useUnityCounter()) {
            // copied from Telegram desktop
            const auto launcherUrl = "application://org.kde.neochat.desktop";
            // Gnome requires that count is a 64bit integer
            const qint64 counterSlice = std::min(m_notificationCount, 9999);
            QVariantMap dbusUnityProperties;

            if (counterSlice > 0) {
                dbusUnityProperties["count"] = counterSlice;
                dbusUnityProperties["count-visible"] = true;
            } else {
                dbusUnityProperties["count-visible"] = false;
            }

            auto signal = QDBusMessage::createSignal(
                "/com/canonical/unity/launcherentry/org.kde.neochat",
                "com.canonical.Unity.LauncherEntry",
                "Update");

            signal.setArguments({
                launcherUrl,
                dbusUnityProperties
            });

            QDBusConnection::sessionBus().send(signal);
        }
    });
#endif
}

RoomListModel::~RoomListModel() = default;

void RoomListModel::setConnection(Connection *connection)
{
    if (connection == m_connection) {
        return;
    }
    if (m_connection) {
        m_connection->disconnect(this);
    }
    if (!connection) {
        qDebug() << "Removing current connection...";
        m_connection = nullptr;
        beginResetModel();
        m_rooms.clear();
        endResetModel();
        return;
    }

    m_connection = connection;

    for (NeoChatRoom *room : qAsConst(m_rooms)) {
        room->disconnect(this);
    }

    connect(connection, &Connection::connected, this, &RoomListModel::doResetModel);
    connect(connection, &Connection::invitedRoom, this, &RoomListModel::updateRoom);
    connect(connection, &Connection::joinedRoom, this, &RoomListModel::updateRoom);
    connect(connection, &Connection::leftRoom, this, &RoomListModel::updateRoom);
    connect(connection, &Connection::aboutToDeleteRoom, this, &RoomListModel::deleteRoom);
    connect(connection, &Connection::directChatsListChanged, this, [=](Quotient::DirectChatsMap additions, Quotient::DirectChatsMap removals) {
        auto refreshRooms = [this, &connection](Quotient::DirectChatsMap rooms) {
            for (const QString &roomID : qAsConst(rooms)) {
                auto room = connection->room(roomID);
                if (room) {
                    refresh(static_cast<NeoChatRoom *>(room));
                }
            }
        };

        refreshRooms(std::move(additions));
        refreshRooms(std::move(removals));
    });

    doResetModel();

    Q_EMIT connectionChanged();
}

void RoomListModel::doResetModel()
{
    beginResetModel();
    m_rooms.clear();
    const auto rooms = m_connection->allRooms();
    for (const auto &room : rooms) {
        doAddRoom(room);
    }
    endResetModel();
    refreshNotificationCount();
}

NeoChatRoom *RoomListModel::roomAt(int row) const
{
    return m_rooms.at(row);
}

void RoomListModel::doAddRoom(Room *r)
{
    if (auto room = static_cast<NeoChatRoom *>(r)) {
        m_rooms.append(room);
        connectRoomSignals(room);
        Q_EMIT roomAdded(room);
    } else {
        qCritical() << "Attempt to add nullptr to the room list";
        Q_ASSERT(false);
    }
}

void RoomListModel::connectRoomSignals(NeoChatRoom *room)
{
    connect(room, &Room::displaynameChanged, this, [=] {
        refresh(room);
    });
    connect(room, &Room::unreadMessagesChanged, this, [=] {
        refresh(room);
    });
    connect(room, &Room::notificationCountChanged, this, [=] {
        refresh(room);
    });
    connect(room, &Room::avatarChanged, this, [this, room] {
        refresh(room, {AvatarRole});
    });
    connect(room, &Room::tagsChanged, this, [=] {
        refresh(room);
    });
    connect(room, &Room::joinStateChanged, this, [=] {
        refresh(room);
    });
    connect(room, &Room::addedMessages, this, [=] {
        refresh(room, {LastEventRole});
    });
    connect(room, &Room::notificationCountChanged, this, [=] {
        if (room->notificationCount() == 0) {
            return;
        }
        if (room->timelineSize() == 0) {
            return;
        }
        auto *lastEvent = room->lastEvent();

        if (!lastEvent) {
            return;
        }

        if (lastEvent->isStateEvent()) {
            return;
        }
        User *sender = room->user(lastEvent->senderId());
        if (sender == room->localUser()) {
            return;
        }
        Q_EMIT newMessage(room->id(), lastEvent->id(), room->displayName(), sender->displayname(), room->eventToString(*lastEvent), room->avatar(128));
    });
    connect(room, &Room::highlightCountChanged, this, [=] {
        if (room->highlightCount() == 0) {
            return;
        }
        if (room->timelineSize() == 0) {
            return;
        }
        auto *lastEvent = room->lastEvent();

        if (!lastEvent) {
            return;
        }

        if (!lastEvent->isStateEvent()) {
            return;
        }
        User *sender = room->user(lastEvent->senderId());
        if (sender == room->localUser()) {
            return;
        }
        Q_EMIT newHighlight(room->id(), lastEvent->id(), room->displayName(), sender->displayname(), room->eventToString(*lastEvent), room->avatar(128));
    });
    connect(room, &Room::notificationCountChanged, this, &RoomListModel::refreshNotificationCount);
}

void RoomListModel::refreshNotificationCount()
{
    int count = 0;
    for (auto room : qAsConst(m_rooms)) {
        count += room->notificationCount();
    }
    if (m_notificationCount == count) {
        return;
    }
    m_notificationCount = count;
    Q_EMIT notificationCountChanged();
}

void RoomListModel::updateRoom(Room *room, Room *prev)
{
    // There are two cases when this method is called:
    // 1. (prev == nullptr) adding a new room to the room list
    // 2. (prev != nullptr) accepting/rejecting an invitation or inviting to
    //    the previously left room (in both cases prev has the previous state).
    if (prev == room) {
        qCritical() << "RoomListModel::updateRoom: room tried to replace itself";
        refresh(static_cast<NeoChatRoom *>(room));
        return;
    }
    if (prev && room->id() != prev->id()) {
        qCritical() << "RoomListModel::updateRoom: attempt to update room" << room->id() << "to" << prev->id();
        // That doesn't look right but technically we still can do it.
    }
    // Ok, we're through with pre-checks, now for the real thing.
    auto newRoom = static_cast<NeoChatRoom *>(room);
    const auto it = std::find_if(m_rooms.begin(), m_rooms.end(), [=](const NeoChatRoom *r) {
        return r == prev || r == newRoom;
    });
    if (it != m_rooms.end()) {
        const int row = it - m_rooms.begin();
        // There's no guarantee that prev != newRoom
        if (*it == prev && *it != newRoom) {
            prev->disconnect(this);
            m_rooms.replace(row, newRoom);
            connectRoomSignals(newRoom);
        }
        Q_EMIT dataChanged(index(row), index(row));
    } else {
        beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
        doAddRoom(newRoom);
        endInsertRows();
    }
}

void RoomListModel::deleteRoom(Room *room)
{
    qDebug() << "Deleting room" << room->id();
    const auto it = std::find(m_rooms.begin(), m_rooms.end(), room);
    if (it == m_rooms.end()) {
        return; // Already deleted, nothing to do
    }
    qDebug() << "Erasing room" << room->id();
    const int row = it - m_rooms.begin();
    beginRemoveRows(QModelIndex(), row, row);
    m_rooms.erase(it);
    endRemoveRows();
}

int RoomListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rooms.count();
}

QVariant RoomListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_rooms.count()) {
        qDebug() << "UserListModel: something wrong here...";
        return QVariant();
    }
    NeoChatRoom *room = m_rooms.at(index.row());
    if (role == NameRole) {
        return !room->name().isEmpty() ? room->name() : room->displayName();
    }
    if (role == DisplayNameRole) {
        return room->displayName();
    }
    if (role == AvatarRole) {
        return room->avatarMediaId();
    }
    if (role == TopicRole) {
        return room->topic();
    }
    if (role == CategoryRole) {
        if (room->joinState() == JoinState::Invite) {
            return RoomType::Invited;
        }
        if (room->isFavourite()) {
            return RoomType::Favorite;
        }
        if (room->isDirectChat()) {
            return RoomType::Direct;
        }
        if (room->isLowPriority()) {
            return RoomType::Deprioritized;
        }
        return RoomType::Normal;
    }
    if (role == UnreadCountRole) {
        return room->unreadCount();
    }
    if (role == NotificationCountRole) {
        return room->notificationCount();
    }
    if (role == HighlightCountRole) {
        return room->highlightCount();
    }
    if (role == LastEventRole) {
        return room->lastEventToString();
    }
    if (role == LastActiveTimeRole) {
        return room->lastActiveTime();
    }
    if (role == JoinStateRole) {
        if (!room->successorId().isEmpty()) {
            return QStringLiteral("upgraded");
        }
        return toCString(room->joinState());
    }
    if (role == CurrentRoomRole) {
        return QVariant::fromValue(room);
    }
    if (role == CategoryVisibleRole) {
        return m_categoryVisibility.value(data(index, CategoryRole).toInt(), true);
    }
    return QVariant();
}

void RoomListModel::refresh(NeoChatRoom *room, const QVector<int> &roles)
{
    const auto it = std::find(m_rooms.begin(), m_rooms.end(), room);
    if (it == m_rooms.end()) {
        qCritical() << "Room" << room->id() << "not found in the room list";
        return;
    }
    const auto idx = index(it - m_rooms.begin());
    Q_EMIT dataChanged(idx, idx, roles);
}

QHash<int, QByteArray> RoomListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[DisplayNameRole] = "displayName";
    roles[AvatarRole] = "avatar";
    roles[TopicRole] = "topic";
    roles[CategoryRole] = "category";
    roles[UnreadCountRole] = "unreadCount";
    roles[NotificationCountRole] = "notificationCount";
    roles[HighlightCountRole] = "highlightCount";
    roles[LastEventRole] = "lastEvent";
    roles[LastActiveTimeRole] = "lastActiveTime";
    roles[JoinStateRole] = "joinState";
    roles[CurrentRoomRole] = "currentRoom";
    roles[CategoryVisibleRole] = "categoryVisible";
    return roles;
}

QString RoomListModel::categoryName(int section)
{
    switch (section) {
    case 1:
        return i18n("Invited");
    case 2:
        return i18n("Favorite");
    case 3:
        return i18n("Direct Messages");
    case 4:
        return i18n("Normal");
    case 5:
        return i18n("Low priority");
    default:
        return "Deadbeef";
    }
}

void RoomListModel::setCategoryVisible(int category, bool visible)
{
    beginResetModel();
    auto collapsedSections = NeoChatConfig::collapsedSections();
    if (visible) {
        collapsedSections.removeAll(category);
    } else {
        collapsedSections.push_back(category);
    }
    NeoChatConfig::setCollapsedSections(collapsedSections);
    NeoChatConfig::self()->save();

    m_categoryVisibility[category] = visible;
    endResetModel();
}

bool RoomListModel::categoryVisible(int category) const
{
    return m_categoryVisibility.value(category, true);
}
