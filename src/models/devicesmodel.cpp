// SPDX-FileCopyrightText: Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "devicesmodel.h"

#include "jobs/neochatdeletedevicejob.h"

#include <QDateTime>
#include <QLocale>

#include <KLocalizedString>

#include <Quotient/csapi/device_management.h>
#include <Quotient/connection.h>
#include <Quotient/user.h>

using namespace Quotient;

DevicesModel::DevicesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(m_connection, &Connection::sessionVerified, this, [this](const QString &, const QString &deviceId) {
        const auto it = std::find_if(m_devices.begin(), m_devices.end(), [deviceId](const Quotient::Device &device) {
            return device.deviceId == deviceId;
        });
        if (it != m_devices.end()) {
            const auto index = this->index(it - m_devices.begin());
            Q_EMIT dataChanged(index, index, {Type});
        }
    });
}

void DevicesModel::fetchDevices()
{
    if (m_connection) {
        auto job = m_connection->callApi<GetDevicesJob>();
        connect(job, &BaseJob::success, this, [this, job]() {
            beginResetModel();
            m_devices = job->devices();
            endResetModel();
            Q_EMIT countChanged();
        });
    }
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount(QModelIndex())) {
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
        } else {
            return false;
        }
    case TimestampString:
        if (device.lastSeenTs) {
            return QDateTime::fromMSecsSinceEpoch(*device.lastSeenTs).toString(QLocale().dateTimeFormat(QLocale::ShortFormat));
        } else {
            return false;
        }
    case Type:
        if (device.deviceId == m_connection->deviceId()) {
            return This;
        }
        if (!m_connection->isKnownE2eeCapableDevice(m_connection->userId(), device.deviceId)) {
            return Unencrypted;
        }
        if (m_connection->isVerifiedDevice(m_connection->userId(), device.deviceId)) {
            return Verified;
        } else {
            return Unverified;
        }
    }
    return {};
}

int DevicesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
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

void DevicesModel::logout(const QString &deviceId, const QString &password)
{
    int index;
    for (index = 0; m_devices[index].deviceId != deviceId; index++)
        ;

    auto job = m_connection->callApi<NeochatDeleteDeviceJob>(m_devices[index].deviceId);

    connect(job, &BaseJob::result, this, [this, job, password, index] {
        auto onSuccess = [this, index]() {
            beginRemoveRows(QModelIndex(), index, index);
            m_devices.remove(index);
            endRemoveRows();
            Q_EMIT countChanged();
        };
        if (job->error() != BaseJob::Success) {
            QJsonObject replyData = job->jsonData();
            QJsonObject authData;
            authData["session"_ls] = replyData["session"_ls];
            authData["password"_ls] = password;
            authData["type"_ls] = "m.login.password"_ls;
            QJsonObject identifier = {{"type"_ls, "m.id.user"_ls}, {"user"_ls, m_connection->user()->id()}};
            authData["identifier"_ls] = identifier;
            auto *innerJob = m_connection->callApi<NeochatDeleteDeviceJob>(m_devices[index].deviceId, authData);
            connect(innerJob, &BaseJob::success, this, onSuccess);
        } else {
            onSuccess();
        }
    });
}

void DevicesModel::setName(const QString &deviceId, const QString &name)
{
    int index;
    for (index = 0; m_devices[index].deviceId != deviceId; index++);

    auto job = m_connection->callApi<UpdateDeviceJob>(m_devices[index].deviceId, name);
    QString oldName = m_devices[index].displayName;
    beginResetModel();
    m_devices[index].displayName = name;
    endResetModel();
    connect(job, &BaseJob::failure, this, [this, index, oldName]() {
        beginResetModel();
        m_devices[index].displayName = oldName;
        endResetModel();
    });
}

Connection *DevicesModel::connection() const
{
    return m_connection;
}

void DevicesModel::setConnection(Connection *connection)
{
    if (m_connection) {
        disconnect(m_connection, nullptr, this, nullptr);
    }
    m_connection = connection;
    Q_EMIT connectionChanged();
    fetchDevices();

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

#include "moc_devicesmodel.cpp"
