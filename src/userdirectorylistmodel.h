// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QObject>

#include <csapi/users.h>

namespace Quotient
{
class Connection;
}

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

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = NameRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] Quotient::Connection *connection() const
    {
        return m_connection;
    }
    void setConnection(Quotient::Connection *conn);

    [[nodiscard]] QString keyword() const
    {
        return m_keyword;
    }
    void setKeyword(const QString &value);

    [[nodiscard]] bool limited() const
    {
        return m_limited;
    }

    Q_INVOKABLE void search(int count = 50);

private:
    Quotient::Connection *m_connection = nullptr;
    QString m_keyword;
    bool m_limited = false;

    bool attempted = false;

    QVector<Quotient::SearchUserDirectoryJob::User> users;

    Quotient::SearchUserDirectoryJob *job = nullptr;

Q_SIGNALS:
    void connectionChanged();
    void keywordChanged();
    void limitedChanged();
};
