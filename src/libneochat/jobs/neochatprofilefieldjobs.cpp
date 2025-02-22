// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatprofilefieldjobs.h"

using namespace Quotient;

NeoChatGetProfileFieldJob::NeoChatGetProfileFieldJob(const QString &userId, const QString &key)
    : BaseJob(HttpVerb::Get, u"GetProfileFieldJob"_s, makePath("/_matrix/client/unstable/uk.tcpip.msc4133", "/profile/", userId, "/", key))
    , m_key(key)
{
}

NeoChatSetProfileFieldJob::NeoChatSetProfileFieldJob(const QString &userId, const QString &key, const QString &value)
    : BaseJob(HttpVerb::Put, u"SetProfileFieldJob"_s, makePath("/_matrix/client/unstable/uk.tcpip.msc4133", "/profile/", userId, "/", key))
{
    QJsonObject _dataJson;
    addParam(_dataJson, key, value);
    setRequestData({_dataJson});
}
