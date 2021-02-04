/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <QAbstractListModel>
#include <QObject>

#include "connection.h"
#include "csapi/definitions/public_rooms_response.h"
#include "csapi/list_public_rooms.h"

using namespace Quotient;

class PublicRoomListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged)
    Q_PROPERTY(QString keyword READ keyword WRITE setKeyword NOTIFY keywordChanged)
    Q_PROPERTY(bool hasMore READ hasMore NOTIFY hasMoreChanged)

public:
    enum EventRoles {
        NameRole = Qt::DisplayRole + 1,
        AvatarRole,
        TopicRole,
        RoomIDRole,
        AliasRole,
        MemberCountRole,
        AllowGuestsRole,
        WorldReadableRole,
        IsJoinedRole,
    };

    PublicRoomListModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = NameRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] Connection *connection() const
    {
        return m_connection;
    }
    void setConnection(Connection *conn);

    [[nodiscard]] QString server() const
    {
        return m_server;
    }
    void setServer(const QString &value);

    [[nodiscard]] QString keyword() const
    {
        return m_keyword;
    }
    void setKeyword(const QString &value);

    [[nodiscard]] bool hasMore() const;

    Q_INVOKABLE void next(int count = 50);

private:
    Connection *m_connection = nullptr;
    QString m_server;
    QString m_keyword;

    bool attempted = false;
    QString nextBatch;

    QVector<PublicRoomsChunk> rooms;

    QueryPublicRoomsJob *job = nullptr;

Q_SIGNALS:
    void connectionChanged();
    void serverChanged();
    void keywordChanged();
    void hasMoreChanged();
};
