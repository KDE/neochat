// SPDX-FileCopyrightText: Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>

#include <csapi/definitions/client_device.h>

namespace Quotient
{
class Connection;
}

class DevicesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(Quotient::Connection *connection READ connection NOTIFY connectionChanged)

public:
    enum Roles {
        Id,
        DisplayName,
        LastIp,
        LastTimestamp,
    };
    Q_ENUM(Roles);

    DevicesModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;

    Q_INVOKABLE void logout(int index, const QString &password);
    Q_INVOKABLE void setName(int index, const QString &name);

    Quotient::Connection *connection() const;

Q_SIGNALS:
    void connectionChanged();

private:
    void fetchDevices();
    QVector<Quotient::Device> m_devices;
};
