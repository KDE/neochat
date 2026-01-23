// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>

#include <Quotient/csapi/list_public_rooms.h>

class NeoChatConnection;

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
    QML_ELEMENT

    /**
     * @brief The current connection that the model is getting its rooms from.
     */
    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The server to get the public room list from.
     */
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged)

    /**
     * @brief The text to search the public room list for.
     */
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

    /**
     * @brief Whether only space rooms should be shown.
     */
    Q_PROPERTY(bool showOnlySpaces READ showOnlySpaces WRITE setShowOnlySpaces NOTIFY showOnlySpacesChanged)

    /**
     * @brief Whether the model is searching.
     */
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)

    /**
     * @brief The text returned by the server after redirection (after performing a search)
     */
    Q_PROPERTY(QString redirectedText READ redirectedText NOTIFY redirectedChanged)

    /**
     * @brief The error while trying to list the rooms (not after searching, which is covered by redirectedText)
     *
     * This is useful when a server completely shut off room search, or any kind of network error.
     */
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        DisplayNameRole = Qt::DisplayRole + 1, /**< The name of the room. */
        AvatarUrlRole, /**< The source URL for the room's avatar. */
        TopicRole, /**< The room topic. */
        RoomIdRole, /**< The room matrix ID. */
        AliasRole, /**< The room canonical alias. */
        MemberCountRole, /**< The number of members in the room. */
        AllowGuestsRole, /**< Whether the room allows guest users. */
        WorldReadableRole, /**< Whether the room events can be seen by non-members. */
        IsJoinedRole, /**< Whether the local user has joined the room. */
        IsSpaceRole, /**< Whether the room is a space. */
    };

    explicit PublicRoomListModel(QObject *parent = nullptr);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = DisplayNameRole) const override;

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

    [[nodiscard]] NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

    [[nodiscard]] QString server() const;
    void setServer(const QString &value);

    [[nodiscard]] QString searchText() const;
    void setSearchText(const QString &searchText);

    [[nodiscard]] bool showOnlySpaces() const;
    void setShowOnlySpaces(bool showOnlySpaces);

    [[nodiscard]] bool searching() const;

    /**
     * @brief Search the room directory.
     *
     * @param limit the maximum number of rooms to load.
     */
    Q_INVOKABLE void search(int limit = 50);

    QString redirectedText() const;

    QString errorText() const;

private:
    QPointer<NeoChatConnection> m_connection = nullptr;
    QString m_server;
    QString m_searchText;
    bool m_showOnlySpaces = false;

    /**
     * @brief Load the next set of rooms.
     *
     * @param limit the maximum number of rooms to load.
     */
    void next(int limit = 50);
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    bool attempted = false;
    bool m_searching = false;
    QString nextBatch;

    QList<Quotient::PublicRoomsChunk> rooms;

    Quotient::QueryPublicRoomsJob *job = nullptr;
    QString m_redirectedText;
    QString m_errorText;

Q_SIGNALS:
    void connectionChanged();
    void serverChanged();
    void searchTextChanged();
    void showOnlySpacesChanged();
    void searchingChanged();
    void redirectedChanged();
    void errorTextChanged();
};
