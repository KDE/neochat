// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <events/stateevent.h>
#include <quotient_common.h>

namespace Quotient
{
#ifdef QUOTIENT_07
class JoinRulesEvent : public StateEvent
#else
class JoinRulesEvent : public StateEventBase
#endif
{
public:
#ifdef QUOTIENT_07
    QUO_EVENT(JoinRulesEvent, "m.room.join_rules")
#else
    DEFINE_EVENT_TYPEID("m.room.join_rules", JoinRulesEvent)
#endif

    explicit JoinRulesEvent(const QJsonObject &obj)
#ifdef QUOTIENT_07
        : StateEvent(obj)
#else
        : StateEventBase(typeId(), obj)
#endif
    {
    }

    QString joinRule() const;
    QJsonArray allow() const;
};
REGISTER_EVENT_TYPE(JoinRulesEvent)
} // namespace Quotient
