// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <csapi/list_public_rooms.h>

#include <QAbstractListModel>
#include <QPointer>
#include <QUrl>

class ServerListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    struct Server {
        QString url;
        bool isHomeServer;
        bool isAddServerDelegate;
        bool isDeletable;
    };
    enum EventRoles {
        UrlRole = Qt::UserRole + 1,
        IsHomeServerRole,
        IsAddServerDelegateRole,
        IsDeletableRole,
    };

    ServerListModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void checkServer(const QString &url);
    Q_INVOKABLE void addServer(const QString &url);
    Q_INVOKABLE void removeServerAtIndex(int index);

Q_SIGNALS:
    void serverCheckComplete(QString url, bool valid);

private:
    QList<Server> m_servers;
    QPointer<Quotient::QueryPublicRoomsJob> m_checkServerJob = nullptr;
};
