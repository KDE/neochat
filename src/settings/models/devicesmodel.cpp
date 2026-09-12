// SPDX-FileCopyrightText: Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "devicesmodel.h"

#include <QDateTime>
#include <QLocale>

#include <KLocalizedString>

#include <Quotient/csapi/device_management.h>
#include <Quotient/user.h>

#include "neochatconnection.h"

using namespace Quotient;

DevicesModel::DevicesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void DevicesModel::fetchDevices()
{
    beginResetModel();
    m_devices.clear();
    endResetModel();

    if (!m_connection) {
        return;
    }

    auto connection = m_connection;
    m_connection->callApi<GetDevicesJob>().onResult(this, [this, connection](const auto &job) {
        if (connection != m_connection) {
            return;
        }
        beginResetModel();
        m_devices = job->devices();
        endResetModel();
        Q_EMIT countChanged();
    });
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        qWarning() << Q_FUNC_INFO << "called with invalid index" << index << role;
        return {};
    }

    if (!m_connection) {
        qWarning() << Q_FUNC_INFO << "caleld nullptr connection";
        return {};
    }

    const auto &device = m_devices[index.row()];

    switch (role) {
    case Id:
        return device.deviceId;
    case DisplayName:
        return device.displayName;
    case LastIp:
        return device.lastSeenIp;
    case LastTimestamp:
        if (device.lastSeenTs) {
            return *device.lastSeenTs;
        }
        break;
    case TimestampString:
        if (device.lastSeenTs) {
            return QDateTime::fromMSecsSinceEpoch(*device.lastSeenTs).toString(QLocale().dateTimeFormat(QLocale::ShortFormat));
        }
        break;
    case Type:
        if (device.deviceId == m_connection->deviceId()) {
            return This;
        }
        if (!m_connection->isKnownE2eeCapableDevice(m_connection->userId(), device.deviceId)) {
            return Unencrypted;
        }
        if (m_connection->isVerifiedDevice(m_connection->userId(), device.deviceId)) {
            return Verified;
        }
        return Unverified;
    }
    return {};
}

int DevicesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_devices.size();
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    return {
        {Id, "id"},
        {DisplayName, "displayName"},
        {LastIp, "lastIp"},
        {LastTimestamp, "lastTimestamp"},
        {TimestampString, "timestamp"},
        {Type, "type"},
    };
}

std::optional<int> DevicesModel::findDevice(const QString &deviceId) const
{
    const auto device = std::ranges::find_if(m_devices, [deviceId](const auto &device) {
        return device.deviceId == deviceId;
    });
    if (device == m_devices.end()) {
        qWarning() << "Device" << deviceId << "not found";
        return std::nullopt;
    }

    return static_cast<int>(std::distance(m_devices.begin(), device));
}

void DevicesModel::logout(const QString &deviceId, const QString &password)
{
    if (!m_connection) {
        qWarning() << Q_FUNC_INFO << "called nullptr connection";
        return;
    }

    // Need to store the connection since it could be switched while the first job is running
    auto connection = m_connection;

    auto onSuccess = [this, connection, deviceId] {
        if (connection != m_connection) {
            return;
        }
        const auto maybeIndex = findDevice(deviceId);
        if (!maybeIndex.has_value()) {
            return;
        }
        const auto index = *maybeIndex;
        beginRemoveRows(QModelIndex(), index, index);
        m_devices.remove(index);
        endRemoveRows();
        Q_EMIT countChanged();
    };
    connection->callApi<DeleteDeviceJob>(deviceId).then(this, onSuccess, [password, connection, deviceId, onSuccess, this](const auto &job) {
        if (!connection) {
            return;
        }
        QJsonObject replyData = job->jsonData();
        AuthenticationData authData;
        authData.session = replyData["session"_L1].toString();
        authData.authInfo["password"_L1] = password;
        authData.type = "m.login.password"_L1;
        authData.authInfo["identifier"_L1] = QJsonObject{{"type"_L1, "m.id.user"_L1}, {"user"_L1, connection->user()->id()}};
        connection->callApi<DeleteDeviceJob>(deviceId, authData).onResult(this, onSuccess);
    });
}

void DevicesModel::setName(const QString &deviceId, const QString &name)
{
    if (!m_connection) {
        qWarning() << Q_FUNC_INFO << "called with nullptr connection";
        return;
    }

    const auto maybeIndex = findDevice(deviceId);
    if (!maybeIndex.has_value()) {
        return;
    }
    const auto index = *maybeIndex;

    auto connection = m_connection;
    auto job = connection->callApi<UpdateDeviceJob>(deviceId, name);
    const auto oldName = m_devices[index].displayName;
    m_devices[index].displayName = name;
    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {DisplayName});
    connect(job, &BaseJob::failure, this, [this, connection, oldName, deviceId, name] {
        if (connection != m_connection) {
            return;
        }
        const auto maybeIndex = findDevice(deviceId);
        if (!maybeIndex.has_value()) {
            return;
        }

        const auto index = *maybeIndex;
        // If the name has meanwhile changed, don't reset it
        if (name != data(this->index(index, 0), DisplayName)) {
            return;
        }
        m_devices[index].displayName = oldName;
        Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {DisplayName});
    });
}

NeoChatConnection *DevicesModel::connection() const
{
    return m_connection;
}

void DevicesModel::setConnection(NeoChatConnection *connection)
{
    if (m_connection) {
        disconnect(m_connection, nullptr, this, nullptr);
    }
    m_connection = connection;
    Q_EMIT connectionChanged();

    fetchDevices();

    if (m_connection) {
        connect(m_connection, &Connection::sessionVerified, this, [this](const QString &userId, const QString &deviceId) {
            Q_UNUSED(deviceId);
            if (userId == m_connection->userId()) {
                fetchDevices();
            }
        });
        connect(m_connection, &Connection::finishedQueryingKeys, this, [this]() {
            fetchDevices();
        });
    }
}

#include "moc_devicesmodel.cpp"
