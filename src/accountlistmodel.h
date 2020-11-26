/**
 * SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef ACCOUNTLISTMODEL_H
#define ACCOUNTLISTMODEL_H

#include "controller.h"

#include <QAbstractListModel>
#include <QObject>

class AccountListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum EventRoles {
        UserRole = Qt::UserRole + 1,
        ConnectionRole,
    };

    AccountListModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = UserRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    QVector<Connection *> m_connections;
};

#endif // ACCOUNTLISTMODEL_H
