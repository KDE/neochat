// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <events/stateevent.h>

namespace Quotient
{
/**
 * @class JoinRulesEvent
 *
 * Class to define a join rule state event.
 *
 * @sa Quotient::StateEvent
 */
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

    /**
     * @brief The join rule for the room.
     *
     * see https://spec.matrix.org/latest/client-server-api/#mroomjoin_rules for
     * the available join rules for a room.
     */
    QString joinRule() const;

    /**
     * @brief The allow rule for restricted rooms.
     *
     * see https://spec.matrix.org/latest/client-server-api/#mroomjoin_rules for
     * full details on allow rules.
     */
    QJsonArray allow() const;
};
REGISTER_EVENT_TYPE(JoinRulesEvent)
}
