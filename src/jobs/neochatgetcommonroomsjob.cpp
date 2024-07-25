// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatgetcommonroomsjob.h"

using namespace Quotient;

NeochatGetCommonRoomsJob::NeochatGetCommonRoomsJob(const QString &userId, const Omittable<QJsonObject> &auth)
    : BaseJob(HttpVerb::Get,
              QStringLiteral("GetCommonRoomsJob"),
              QStringLiteral("/_matrix/client/unstable/uk.half-shot.msc2666/user/mutual_rooms").toLatin1(),
              QUrlQuery({{QStringLiteral("user_id"), userId}}))
{
}
