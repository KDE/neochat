// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "neochatconnection.h"

#include <Quotient/events/roommessageevent.h>
#include <Quotient/roommember.h>

/**
 * @brief Model to show the common or mutual rooms between you and another user.
 */
class CommonRoomsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(NeoChatConnection *connection WRITE setConnection READ connection NOTIFY connectionChanged REQUIRED)
    Q_PROPERTY(QString userId WRITE setUserId READ userId NOTIFY userIdChanged REQUIRED)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    enum Roles {
        RoomIdRole = Qt::UserRole,
        RoomNameRole,
        RoomAvatarRole,
    };
    Q_ENUM(Roles)

    explicit CommonRoomsModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

    [[nodiscard]] QString userId() const;
    void setUserId(const QString &userId);

    [[nodiscard]] QVariant data(const QModelIndex &index, int roleName) const override;
    [[nodiscard]] Q_INVOKABLE int rowCount(const QModelIndex &parent = {}) const override;

    QHash<int, QByteArray> roleNames() const override;

    bool loading() const;

Q_SIGNALS:
    void connectionChanged();
    void userIdChanged();
    void countChanged();
    void loadingChanged();

private:
    void reload();

    QPointer<NeoChatConnection> m_connection;
    QString m_userId;
    QList<QString> m_commonRooms;
    bool m_loading = false;
};
