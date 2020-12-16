/**
 * SPDX-FileCopyrightText: Tobias Fella <fella@posteo.de>
 *
 * SPDX-LicenseIdentifier: GPL-2.0-or-later
 */

#include "devicesmodel.h"

#include <csapi/device_management.h>

#include "controller.h"

DevicesModel::DevicesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    GetDevicesJob *job = Controller::instance().activeConnection()->callApi<GetDevicesJob>();
    connect(job, &BaseJob::success, this, [this, job]() {
        beginResetModel();
        m_devices = job->devices();
        endResetModel();
    });
}

QVariant DevicesModel::data(const QModelIndex& index, int role) const
{
    if(index.row() < 0 || index.row() >= rowCount(QModelIndex()))
        return QVariant();
    switch(role) {
        case Id:
            return m_devices[index.row()].deviceId;
        case DisplayName:
            return m_devices[index.row()].displayName;
        case LastIp:
            return m_devices[index.row()].lastSeenIp;
        case LastTimestamp:
            if(m_devices[index.row()].lastSeenTs)
                return *m_devices[index.row()].lastSeenTs;
    }
    return QVariant();
}

int DevicesModel::rowCount(const QModelIndex& parent) const
{
    return m_devices.size();
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    return {{Id, "id"}, {DisplayName, "displayName"}, {LastIp, "lastIp"}, {LastTimestamp, "lastTimestamp"}};
}

void DevicesModel::logout(int index, const QString &password)
{
    auto job = Controller::instance().activeConnection()->callApi<NeochatDeleteDeviceJob>(m_devices[index].deviceId);

    connect(job, &BaseJob::result, this, [this, job, password, index] {
        if (job->error() != 0) {
            QJsonObject replyData = job->jsonData();
            QJsonObject authData;
            authData["session"] = replyData["session"];
            authData["password"] = password;
            authData["type"] = "m.login.password";
            QJsonObject identifier = {{"type", "m.id.user"}, {"user", Controller::instance().activeConnection()->user()->id()}};
            authData["identifier"] = identifier;
            auto *innerJob = Controller::instance().activeConnection()->callApi<NeochatDeleteDeviceJob>(m_devices[index].deviceId, authData);
            connect(innerJob, &BaseJob::success, this, [this, index]() {
                Q_EMIT beginRemoveRows(QModelIndex(), index, index);
                m_devices.remove(index);
                Q_EMIT endRemoveRows();
            });
        }
    });
}

void DevicesModel::setName(int index, const QString &name)
{
    auto job = Controller::instance().activeConnection()->callApi<UpdateDeviceJob>(m_devices[index].deviceId, name);
    QString oldName = m_devices[index].displayName;
    Q_EMIT beginResetModel();
    m_devices[index].displayName = name;
    Q_EMIT endResetModel();
    connect(job, &BaseJob::failure, this, [=]() {
        Q_EMIT beginResetModel();
        m_devices[index].displayName = oldName;
        Q_EMIT endResetModel();
    });
}
