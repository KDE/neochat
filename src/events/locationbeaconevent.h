// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <Quotient/events/simplestateevents.h>

namespace Quotient
{

// Defined so we can directly switch on type.
DEFINE_SIMPLE_STATE_EVENT(LocationBeaconEvent, "org.matrix.msc3672.beacon_info", QString, body, "body")

} // namespace Quotient
