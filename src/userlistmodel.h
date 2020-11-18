/**
 * SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include "room.h"

#include <QAbstractListModel>
#include <QObject>

namespace Quotient
{
class Connection;
class Room;
class User;
} // namespace Quotient

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
    };
    Q_ENUM(Types)
};

class UserListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Quotient::Room *room READ room WRITE setRoom NOTIFY roomChanged)
public:
    enum EventRoles {
        NameRole = Qt::UserRole + 1,
        UserIDRole,
        AvatarRole,
        ObjectRole,
        PermRole,
    };

    UserListModel(QObject *parent = nullptr);

    Quotient::Room *room() const
    {
        return m_currentRoom;
    }
    void setRoom(Quotient::Room *room);
    Quotient::User *userAt(QModelIndex index) const;

    QVariant data(const QModelIndex &index, int role = NameRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void roomChanged();

private Q_SLOTS:
    void userAdded(Quotient::User *user);
    void userRemoved(Quotient::User *user);
    void refresh(Quotient::User *user, QVector<int> roles = {});
    void avatarChanged(Quotient::User *user, const Quotient::Room *context);

private:
    Quotient::Room *m_currentRoom;
    QList<Quotient::User *> m_users;

    int findUserPos(Quotient::User *user) const;
    int findUserPos(const QString &username) const;
};

#endif // USERLISTMODEL_H
