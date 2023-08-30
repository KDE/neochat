// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatdeactivateaccountjob.h"

using namespace Quotient;

NeoChatDeactivateAccountJob::NeoChatDeactivateAccountJob(const Omittable<QJsonObject> &auth)
    : BaseJob(HttpVerb::Post, QStringLiteral("DisableDeviceJob"), "_matrix/client/v3/account/deactivate")
{
    QJsonObject data;
    addParam<IfNotEmpty>(data, QStringLiteral("auth"), auth);
    setRequestData(data);
}
