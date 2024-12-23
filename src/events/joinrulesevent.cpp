// SPDX-FileCopyrightText: 2019 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "joinrulesevent.h"

using namespace Quotient;

QString JoinRulesEvent::joinRule() const
{
    return fromJson<QString>(contentJson()["join_rule"_L1]);
}

QJsonArray JoinRulesEvent::allow() const
{
    return contentJson()["allow"_L1].toArray();
}
