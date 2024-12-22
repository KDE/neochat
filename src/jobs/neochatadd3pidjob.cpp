// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "neochatadd3pidjob.h"

using namespace Quotient;

NeochatAdd3PIdJob::NeochatAdd3PIdJob(const QString &clientSecret, const QString &sid, const std::optional<QJsonObject> &auth)
    : BaseJob(HttpVerb::Post, u"Add3PIDJob"_s, makePath("/_matrix/client/v3", "/account/3pid/add"))
{
    QJsonObject _dataJson;
    addParam<IfNotEmpty>(_dataJson, u"auth"_s, auth);
    addParam<>(_dataJson, u"client_secret"_s, clientSecret);
    addParam<>(_dataJson, u"sid"_s, sid);
    setRequestData({_dataJson});
}
