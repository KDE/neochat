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
class JoinRulesEvent : public StateEvent
{
public:
    QUO_EVENT(JoinRulesEvent, "m.room.join_rules")

    explicit JoinRulesEvent(const QJsonObject &obj)
        : StateEvent(obj)
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
