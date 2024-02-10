// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <Quotient/events/simplestateevents.h>

namespace Quotient
{

// Defined so we can directly switch on type.
DEFINE_SIMPLE_STATE_EVENT(ServerAclEvent, "m.room.server_acl", bool, allow_ip_literals, "allow_ip_literals")

} // namespace Quotient
