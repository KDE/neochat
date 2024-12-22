// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatdeletedevicejob.h"

using namespace Quotient;

NeochatDeleteDeviceJob::NeochatDeleteDeviceJob(const QString &deviceId, const std::optional<QJsonObject> &auth)
    : BaseJob(HttpVerb::Delete, u"DeleteDeviceJob"_s, u"/_matrix/client/r0/devices/%1"_s.arg(deviceId).toLatin1())
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, u"auth"_s, auth);
    setRequestData(std::move(_data));
}
