// SPDX-FileCopyrightText: 2020 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-FileCopyrightText: Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QObject>

namespace Quotient
{
class Connection;

class AccountRegistry : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int accountCount READ count NOTIFY accountCountChanged);

public:
    enum EventRoles {
        ConnectionRole = Qt::UserRole + 1,
        UserIdRole = Qt::DisplayRole,
    };

    static AccountRegistry &instance()
    {
        static AccountRegistry _instance;
        return _instance;
    }

    const QVector<Connection *> accounts() const;
    void add(Connection *a);
    void drop(Connection *a);
    bool isLoggedIn(const QString &userId) const;
    bool isEmpty() const;
    int count() const;
    bool contains(Connection *) const;
    Connection *get(const QString &userId);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void accountCountChanged();

private:
    AccountRegistry();

    QVector<Connection *> m_accounts;
};
}
