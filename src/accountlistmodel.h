/**
 * SPDX-FileCopyrightText: Black Hat <bhat@encom.eu.org>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
#ifndef ACCOUNTLISTMODEL_H
#define ACCOUNTLISTMODEL_H

#include "controller.h"

#include <QAbstractListModel>
#include <QObject>

class AccountListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Controller *controller READ controller WRITE setController NOTIFY controllerChanged)
public:
    enum EventRoles { UserRole = Qt::UserRole + 1, ConnectionRole };

    AccountListModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = UserRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

    Controller *controller() const
    {
        return m_controller;
    }
    void setController(Controller *value);

private:
    Controller *m_controller = nullptr;
    QVector<Connection *> m_connections;

Q_SIGNALS: 
    void controllerChanged();
};

#endif // ACCOUNTLISTMODEL_H
