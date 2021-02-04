/**
 * SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "connection.h"
#include "events/roomevent.h"
#include "neochatroom.h"
#include "room.h"

#include <QAbstractListModel>

using namespace Quotient;

class RoomType : public QObject
{
    Q_OBJECT

public:
    enum Types {
        Invited = 1,
        Favorite,
        Direct,
        Normal,
        Deprioritized,
    };
    Q_ENUM(Types)
};

class RoomListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(int notificationCount READ notificationCount NOTIFY notificationCountChanged)

public:
    enum EventRoles {
        NameRole = Qt::UserRole + 1,
        DisplayNameRole,
        AvatarRole,
        TopicRole,
        CategoryRole,
        UnreadCountRole,
        NotificationCountRole,
        HighlightCountRole,
        LastEventRole,
        LastActiveTimeRole,
        JoinStateRole,
        CurrentRoomRole,
        CategoryVisibleRole,
    };
    Q_ENUM(EventRoles)

    RoomListModel(QObject *parent = nullptr);
    ~RoomListModel() override;

    [[nodiscard]] Connection *connection() const
    {
        return m_connection;
    }
    void setConnection(Connection *connection);
    void doResetModel();

    Q_INVOKABLE [[nodiscard]] NeoChatRoom *roomAt(int row) const;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE [[nodiscard]] static QString categoryName(int section);
    Q_INVOKABLE void setCategoryVisible(int category, bool visible);
    Q_INVOKABLE [[nodiscard]] bool categoryVisible(int category) const;

    [[nodiscard]] int notificationCount() const
    {
        return m_notificationCount;
    }

    Q_INVOKABLE NeoChatRoom *roomByAliasOrId(const QString &aliasOrId);

private Q_SLOTS:
    void doAddRoom(Quotient::Room *room);
    void updateRoom(Quotient::Room *room, Quotient::Room *prev);
    void deleteRoom(Quotient::Room *room);
    void refresh(NeoChatRoom *room, const QVector<int> &roles = {});
    void refreshNotificationCount();

private:
    Connection *m_connection = nullptr;
    QList<NeoChatRoom *> m_rooms;

    QMap<int, bool> m_categoryVisibility;

    int m_notificationCount = 0;

    void connectRoomSignals(NeoChatRoom *room);

Q_SIGNALS:
    void connectionChanged();
    void notificationCountChanged();

    void roomAdded(NeoChatRoom *_t1);
    void newMessage(const QString &_t1, const QString &_t2, const QString &_t3, const QString &_t4, const QString &_t5, const QImage &_t6);
    void newHighlight(const QString &_t1, const QString &_t2, const QString &_t3, const QString &_t4, const QString &_t5, const QImage &_t6);
};
