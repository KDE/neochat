// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatdeactivateaccountjob.h"

using namespace Quotient;

NeoChatDeactivateAccountJob::NeoChatDeactivateAccountJob(const std::optional<QJsonObject> &auth)
    : BaseJob(HttpVerb::Post, u"DisableDeviceJob"_s, "_matrix/client/v3/account/deactivate")
{
    QJsonObject data;
    addParam<IfNotEmpty>(data, u"auth"_s, auth);
    setRequestData(data);
}
