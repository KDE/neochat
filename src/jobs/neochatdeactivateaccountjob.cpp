// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatdeactivateaccountjob.h"

using namespace Quotient;

NeoChatDeactivateAccountJob::NeoChatDeactivateAccountJob(const std::optional<QJsonObject> &auth, const bool erase)
    : BaseJob(HttpVerb::Post, u"DisableDeviceJob"_s, "_matrix/client/v3/account/deactivate")
{
    QJsonObject data;
    addParam<IfNotEmpty>(data, u"auth"_s, auth);
    addParam<IfNotEmpty>(data, u"erase"_s, erase);
    setRequestData(data);
}
