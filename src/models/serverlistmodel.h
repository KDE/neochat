// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <Quotient/csapi/list_public_rooms.h>

#include <QAbstractListModel>
#include <QPointer>
#include <QQmlEngine>
#include <QUrl>

class NeoChatConnection;

/**
 * @class ServerListModel
 *
 * This class defines the model for visualising a list of matrix servers.
 *
 * The list of servers is retrieved from the local cache. Any additions are also
 * stored locally so that they are retrieved on subsequent instantiations.
 *
 * The model also automatically adds the local user's home server and matrix.org to
 * the model. Finally the model also adds an entry to create a space in the model
 * for an "add new server" delegate.
 */
class ServerListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

public:
    /**
     * @brief Define the data required to represent a server.
     */
    struct Server {
        QString url; /**< Server URL. */
        bool isHomeServer; /**< Whether the server is the local user's home server. */
        bool isAddServerDelegate; /**< Wether the item is the "add new server" delegate. */
        bool isDeletable; /**< Whether the item can be deleted from the model. */
    };

    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        UrlRole = Qt::UserRole + 1, /**< Server URL. */
        IsHomeServerRole, /**< Whether the server is the local user's home server. */
        IsAddServerDelegateRole, /**< Whether the item is the add new server delegate. */
        IsDeletableRole, /**< Whether the item can be deleted from the model. */
    };

    explicit ServerListModel(QObject *parent = nullptr);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Start a check to see if the given URL is a valid matrix server.
     *
     * This function starts the check but due to the requests being asynchronous
     * the caller will need to watch the serverCheckComplete signal for confirmation.
     * The server URL should be treated as invalid until the signal is emitted true.
     *
     * @sa serverCheckComplete()
     */
    Q_INVOKABLE void checkServer(const QString &url);

    /**
     * @brief Add a new server to the model.
     *
     * The server will also be stored in local cache.
     */
    Q_INVOKABLE void addServer(const QString &url);

    /**
     * @brief Remove the server at the given index.
     *
     * The server will also be removed from local cache.
     */
    Q_INVOKABLE void removeServerAtIndex(int index);

    NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

Q_SIGNALS:
    void serverCheckComplete(QString url, bool valid);
    void connectionChanged();

private:
    QList<Server> m_servers;
    QPointer<Quotient::QueryPublicRoomsJob> m_checkServerJob = nullptr;
    NeoChatConnection *m_connection = nullptr;
};
