// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <events/stateevent.h>
#include <quotient_common.h>

namespace Quotient
{
class JoinRulesEvent : public StateEventBase
{
public:
    DEFINE_EVENT_TYPEID("m.room.join_rules", JoinRulesEvent)

    explicit JoinRulesEvent()
        : StateEventBase(typeId(), matrixTypeId())
    {
    }
    explicit JoinRulesEvent(const QJsonObject &obj)
        : StateEventBase(typeId(), obj)
    {
    }

    QString joinRule() const;
    QJsonArray allow() const;
};
REGISTER_EVENT_TYPE(JoinRulesEvent)
} // namespace Quotient
