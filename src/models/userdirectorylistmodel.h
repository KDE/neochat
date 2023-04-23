// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QObject>

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

    /**
     * @brief The current connection that the model is getting users from.
     */
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The keyword to use in the search.
     */
    Q_PROPERTY(QString keyword READ keyword WRITE setKeyword NOTIFY keywordChanged)

    /**
     * @brief Whether the current results have been truncated.
     */
    Q_PROPERTY(bool limited READ limited NOTIFY limitedChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        NameRole = Qt::DisplayRole + 1, /**< The user's display name. */
        AvatarRole, /**< The source URL for the user's avatar. */
        UserIDRole, /**< Matrix ID of the user. */
        DirectChatsRole, /**< A list of direct chat matrix IDs with the user. */
    };

    explicit UserDirectoryListModel(QObject *parent = nullptr);

    [[nodiscard]] Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *conn);

    [[nodiscard]] QString keyword() const;
    void setKeyword(const QString &value);

    [[nodiscard]] bool limited() const;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = NameRole) const override;

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
     * @brief Start the user search.
     */
    Q_INVOKABLE void search(int count = 50);

Q_SIGNALS:
    void connectionChanged();
    void keywordChanged();
    void limitedChanged();

private:
    Quotient::Connection *m_connection = nullptr;
    QString m_keyword;
    bool m_limited = false;

    bool attempted = false;

    QVector<Quotient::SearchUserDirectoryJob::User> users;

    Quotient::SearchUserDirectoryJob *job = nullptr;
};
