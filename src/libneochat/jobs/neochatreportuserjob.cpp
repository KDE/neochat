// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatreportuserjob.h"

using namespace Quotient;

NeochatReportUserJob::NeochatReportUserJob(const QString &userId, const QString &reason)
    : BaseJob(HttpVerb::Post, u"ReportUserJob"_s, makePath("/_matrix/client/v3/users/", userId, "/report"))
{
    QJsonObject _dataJson;
    addParam<IfNotEmpty>(_dataJson, "reason"_L1, reason);
    setRequestData({_dataJson});
}
