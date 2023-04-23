// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QObject>

#include <Quotient/csapi/list_public_rooms.h>

namespace Quotient
{
class Connection;
}

/**
 * @class PublicRoomListModel
 *
 * This class defines the model for visualising a list of public rooms.
 *
 * The model finds the public rooms visible to the given server (which doesn't have
 * to be the user's home server) and can also apply a filter if desired.
 *
 * Due to the fact that the public room list could be huge the model is lazily loaded
 * and requires that the next batch of rooms be manually called.
 */
class PublicRoomListModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The current connection that the model is getting its rooms from.
     */
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The server to get the public room list from.
     */
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged)

    /**
     * @brief The filter keyword for the list of public rooms.
     */
    Q_PROPERTY(QString keyword READ keyword WRITE setKeyword NOTIFY keywordChanged)

    /**
     * @brief Whether the model has more items to load.
     */
    Q_PROPERTY(bool hasMore READ hasMore NOTIFY hasMoreChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        NameRole = Qt::DisplayRole + 1, /**< The name of the room. */
        AvatarRole, /**< The source URL for the room's avatar. */
        TopicRole, /**< The room topic. */
        RoomIDRole, /**< The room matrix ID. */
        AliasRole, /**< The room canonical alias. */
        MemberCountRole, /**< The number of members in the room. */
        AllowGuestsRole, /**< Whether the room allows guest users. */
        WorldReadableRole, /**< Whether the room events can be seen by non-members. */
        IsJoinedRole, /**< Whether the local user has joined the room. */
    };

    PublicRoomListModel(QObject *parent = nullptr);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = NameRole) const override;

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

    [[nodiscard]] Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *conn);

    [[nodiscard]] QString server() const;
    void setServer(const QString &value);

    [[nodiscard]] QString keyword() const;
    void setKeyword(const QString &value);

    [[nodiscard]] bool hasMore() const;

    /**
     * @brief Load the next set of rooms.
     *
     * @param count the maximum number of rooms to load.
     */
    Q_INVOKABLE void next(int count = 50);

private:
    Quotient::Connection *m_connection = nullptr;
    QString m_server;
    QString m_keyword;

    bool attempted = false;
    QString nextBatch;

    QVector<Quotient::PublicRoomsChunk> rooms;

    Quotient::QueryPublicRoomsJob *job = nullptr;

Q_SIGNALS:
    void connectionChanged();
    void serverChanged();
    void keywordChanged();
    void hasMoreChanged();
};
