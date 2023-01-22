// SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <room.h>

#include <QAbstractListModel>
#include <QObject>

class NeoChatRoom;

namespace Quotient
{
class User;
class Room;
}

class UserType : public QObject
{
    Q_OBJECT

public:
    enum Types {
        Owner = 1,
        Admin,
        Moderator,
        Member,
        Muted,
        Custom,
    };
    Q_ENUM(Types)
};

class UserListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)
public:
    enum EventRoles {
        NameRole = Qt::UserRole + 1,
        UserIdRole,
        AvatarRole,
        ObjectRole,
        PowerLevelRole,
        PowerLevelStringRole,
    };

    UserListModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    [[nodiscard]] Quotient::User *userAt(QModelIndex index) const;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = NameRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void roomChanged();
    void usersRefreshed();

private Q_SLOTS:
    void userAdded(Quotient::User *user);
    void userRemoved(Quotient::User *user);
    void refresh(Quotient::User *user, const QVector<int> &roles = {});
    void refreshAll();
    void avatarChanged(Quotient::User *user, const Quotient::Room *context);

private:
    NeoChatRoom *m_currentRoom;
    QList<Quotient::User *> m_users;

    int findUserPos(Quotient::User *user) const;
    [[nodiscard]] int findUserPos(const QString &username) const;
};
