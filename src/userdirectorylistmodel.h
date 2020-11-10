/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef USERDIRECTORYLISTMODEL_H
#define USERDIRECTORYLISTMODEL_H

#include <QAbstractListModel>
#include <QObject>

#include "connection.h"
#include "csapi/users.h"

using namespace Quotient;

class UserDirectoryListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(QString keyword READ keyword WRITE setKeyword NOTIFY keywordChanged)
    Q_PROPERTY(bool limited READ limited NOTIFY limitedChanged)

public:
    enum EventRoles {
        NameRole = Qt::DisplayRole + 1,
        AvatarRole,
        UserIDRole,
        DirectChatsRole,
    };

    UserDirectoryListModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = NameRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

    Connection *connection() const
    {
        return m_connection;
    }
    void setConnection(Connection *value);

    QString keyword() const
    {
        return m_keyword;
    }
    void setKeyword(const QString &value);

    bool limited() const
    {
        return m_limited;
    }

    Q_INVOKABLE void search(int count = 50);

private:
    Connection *m_connection = nullptr;
    QString m_keyword;
    bool m_limited = false;

    bool attempted = false;

    QVector<SearchUserDirectoryJob::User> users;

    SearchUserDirectoryJob *job = nullptr;

Q_SIGNALS:
    void connectionChanged();
    void keywordChanged();
    void limitedChanged();
};

#endif // USERDIRECTORYLISTMODEL_H
