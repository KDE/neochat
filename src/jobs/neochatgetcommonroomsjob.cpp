// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatgetcommonroomsjob.h"

using namespace Quotient;

NeochatGetCommonRoomsJob::NeochatGetCommonRoomsJob(const QString &userId)
    : BaseJob(HttpVerb::Get, u"GetCommonRoomsJob"_s, "/_matrix/client/unstable/uk.half-shot.msc2666/user/mutual_rooms", QUrlQuery({{u"user_id"_s, userId}}))
{
}
