// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatreportroomjob.h"

using namespace Quotient;

NeochatReportRoomJob::NeochatReportRoomJob(const QString &userId, const QString &reason)
    : BaseJob(HttpVerb::Post, u"ReportRoomJob"_s, makePath(" /_matrix/client/v3/", userId, "/report"))
{
    QJsonObject _dataJson;
    addParam<IfNotEmpty>(_dataJson, "reason"_L1, reason);
    setRequestData({_dataJson});
}
