// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "neochatadd3pidjob.h"

using namespace Quotient;

NeochatAdd3PIdJob::NeochatAdd3PIdJob(const QString &clientSecret, const QString &sid, const Omittable<QJsonObject> &auth)
    : BaseJob(HttpVerb::Post, QStringLiteral("Add3PIDJob"), makePath("/_matrix/client/v3", "/account/3pid/add"))
{
    QJsonObject _dataJson;
    addParam<IfNotEmpty>(_dataJson, QStringLiteral("auth"), auth);
    addParam<>(_dataJson, QStringLiteral("client_secret"), clientSecret);
    addParam<>(_dataJson, QStringLiteral("sid"), sid);
    setRequestData({_dataJson});
}
