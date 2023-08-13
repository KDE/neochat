// SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <Quotient/room.h>

#include <QAbstractListModel>
#include <QObject>
#include <QPointer>

class NeoChatRoom;

namespace Quotient
{
class User;
}

/**
 * @class UserListModel
 *
 * This class defines the model for listing the users in a room.
 *
 * As well as gathering all the users from a room, the model ensures that they are
 * sorted in alphabetical order.
 *
 * @sa NeoChatRoom
 */
class UserListModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The room that the model is getting its users from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        DisplayNameRole = Qt::DisplayRole, /**< The user's display name in the current room. */
        UserIdRole, /**< Matrix ID of the user. */
        AvatarRole, /**< The source URL for the user's avatar in the current room. */
        ObjectRole, /**< The QObject for the user. */
        PowerLevelRole, /**< The user's power level in the current room. */
        PowerLevelStringRole, /**< The name of the user's power level in the current room. */
    };
    Q_ENUM(EventRoles)

    explicit UserListModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void roomChanged();
    void usersRefreshed();

private Q_SLOTS:
    void memberAdded(QString memberId);
    void memberUpdated(QString memberId);
    void memberRemoved(QString memberId);
    void refreshAllUsers();

private:
    QPointer<NeoChatRoom> m_currentRoom;
    QList<QSharedPointer<Quotient::RoomMember>> m_members;
};
