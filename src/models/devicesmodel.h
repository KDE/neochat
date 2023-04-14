// SPDX-FileCopyrightText: Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>

#include <csapi/definitions/client_device.h>

namespace Quotient
{
class Connection;
}

/**
 * @class DevicesModel
 *
 * This class defines the model for managing the devices of the local user.
 *
 * A device is any session where the local user is logged into a client. This means
 * the same physical device can have multiple sessions for example if the user uses
 * multiple clients on the same machine.
 */
class DevicesModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The current connection that the model is getting its devices from.
     */
    Q_PROPERTY(Quotient::Connection *connection READ connection NOTIFY connectionChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        Id, /**< The device ID. */
        DisplayName, /**< Display name set by the user for this device. */
        LastIp, /**< The IP address where this device was last seen. */
        LastTimestamp, /**< The timestamp when this devices was last seen. */
    };
    Q_ENUM(Roles);

    DevicesModel(QObject *parent = nullptr);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Logout the device at the given index.
     */
    Q_INVOKABLE void logout(int index, const QString &password);

    /**
     * @brief Set the display name of the device at the given index.
     */
    Q_INVOKABLE void setName(int index, const QString &name);

    Quotient::Connection *connection() const;

Q_SIGNALS:
    void connectionChanged();

private:
    void fetchDevices();
    QVector<Quotient::Device> m_devices;
};
