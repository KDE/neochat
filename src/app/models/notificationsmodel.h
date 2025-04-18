// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "neochatconnection.h"
#include <QAbstractListModel>
#include <QPointer>
#include <QQmlEngine>
#include <QVariant>
#include <Quotient/csapi/notifications.h>

class NotificationsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString nextToken READ nextToken NOTIFY nextTokenChanged)

public:
    enum Roles {
        TextRole = Qt::DisplayRole,
        RoomIdRole,
        AuthorName,
        AuthorAvatar,
        RoomRole,
        EventIdRole,
        RoomDisplayNameRole,
        UriRole,
    };
    Q_ENUM(Roles);

    struct Notification {
        QString roomId;
        QString text;
        QString authorName;
        QUrl authorAvatar;
        QString eventId;
        QString roomDisplayName;
    };

    NotificationsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);
    bool loading() const;
    QString nextToken() const;

Q_SIGNALS:
    void connectionChanged();
    void loadingChanged();
    void nextTokenChanged();

private:
    QPointer<NeoChatConnection> m_connection;
    void loadData();
    QList<Notification> m_notifications;
    QString m_nextToken;
    QPointer<Quotient::GetNotificationsJob> m_job;
};
