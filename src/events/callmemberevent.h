// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <Quotient/events/stateevent.h>

namespace Quotient
{

struct Focus {
    QString livekitAlias;
    QString livekitServiceUrl;
    QString type;
};

struct CallMembership {
    QString application;
    QString callId;
    QString deviceId;
    int expires;
    uint64_t expiresTs;
    QList<Focus> fociActive;
    QString membershipId;
    QString scope;
};

class CallMemberEventContent
{
public:
    explicit CallMemberEventContent(const QJsonObject &json);
    QJsonObject toJson() const;

    QList<CallMembership> memberships;
};

/**
 * @class CallMemberEvent
 *
 * Class to define a call member event.
 *
 * @sa Quotient::StateEvent
 */
class CallMemberEvent : public KeyedStateEventBase<CallMemberEvent, CallMemberEventContent>
{
public:
    QUO_EVENT(CallMemberEvent, "org.matrix.msc3401.call.member")

    explicit CallMemberEvent(const QJsonObject &obj)
        : KeyedStateEventBase(obj)
    {
    }

    QJsonArray memberships() const
    {
        return contentJson()[u"memberships"_s].toArray();
    }
};
}
