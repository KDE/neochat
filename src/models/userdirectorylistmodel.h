// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>

#include <Quotient/csapi/users.h>

namespace Quotient
{
class Connection;
}

/**
 * @class UserDirectoryListModel
 *
 * This class defines the model for visualising the results of a user search.
 *
 * The model searches for users that have the given keyword in the matrix
 * ID or the display name. See
 * https://spec.matrix.org/v1.6/client-server-api/#post_matrixclientv3user_directorysearch
 * for more info on matrix user searches.
 */
class UserDirectoryListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current connection that the model is getting users from.
     */
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The text to search the public room list for.
     */
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

    /**
     * @brief Whether the model is searching.
     */
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        DisplayNameRole = Qt::DisplayRole, /**< The user's display name. */
        AvatarRole, /**< The source URL for the user's avatar. */
        UserIDRole, /**< Matrix ID of the user. */
        DirectChatExistsRole, /**< Whether there is already a direct chat with the user. */
    };

    explicit UserDirectoryListModel(QObject *parent = nullptr);

    [[nodiscard]] Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *conn);

    [[nodiscard]] QString searchText() const;
    void setSearchText(const QString &searchText);

    [[nodiscard]] bool searching() const;

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
     * @brief Search the user directory.
     *
     * @param limit the maximum number of rooms to load.
     */
    Q_INVOKABLE void search(int limit = 50);

Q_SIGNALS:
    void connectionChanged();
    void searchTextChanged();
    void searchingChanged();

private:
    Quotient::Connection *m_connection = nullptr;
    QString m_searchText;

    bool attempted = false;
    QList<Quotient::SearchUserDirectoryJob::User> users;

    Quotient::SearchUserDirectoryJob *m_job = nullptr;
};
