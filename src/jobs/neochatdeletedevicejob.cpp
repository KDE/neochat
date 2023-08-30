// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatdeletedevicejob.h"

using namespace Quotient;

NeochatDeleteDeviceJob::NeochatDeleteDeviceJob(const QString &deviceId, const Omittable<QJsonObject> &auth)
    : BaseJob(HttpVerb::Delete, QStringLiteral("DeleteDeviceJob"), QStringLiteral("/_matrix/client/r0/devices/%1").arg(deviceId).toLatin1())
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, QStringLiteral("auth"), auth);
    setRequestData(std::move(_data));
}
