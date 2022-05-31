// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <events/stateevent.h>

namespace Quotient
{
class JoinRulesEvent : public StateEvent
{
public:
    QUO_EVENT(JoinRulesEvent, "m.room.join_rules")

    explicit JoinRulesEvent(const QJsonObject &obj)
        : StateEvent(obj)
    {
    }

    QString joinRule() const;
    QJsonArray allow() const;
};
}
