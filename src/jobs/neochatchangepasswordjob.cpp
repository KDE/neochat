// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatchangepasswordjob.h"

using namespace Quotient;

NeochatChangePasswordJob::NeochatChangePasswordJob(const QString &newPassword, bool logoutDevices, const std::optional<QJsonObject> &auth)
    : BaseJob(HttpVerb::Post, u"ChangePasswordJob"_s, "/_matrix/client/r0/account/password")
{
    QJsonObject _data;
    addParam<>(_data, u"new_password"_s, newPassword);
    addParam<IfNotEmpty>(_data, u"logout_devices"_s, logoutDevices);
    addParam<IfNotEmpty>(_data, u"auth"_s, auth);
    setRequestData(_data);
}
