/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "neochatuser.h"

#include "csapi/profile.h"

QColor NeoChatUser::color()
{
    return QColor::fromHslF(hueF(), 0.7, 0.5, 1);
}
