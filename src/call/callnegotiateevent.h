// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <events/callevents.h>

namespace Quotient
{

class CallNegotiateEvent : public EventTemplate<CallNegotiateEvent, CallEvent>
{
public:
    QUO_EVENT(CallNegotiateEvent, "m.call.negotiate")

    explicit CallNegotiateEvent(const QJsonObject &obj);

    explicit CallNegotiateEvent(const QString &callId,
                                const QString &partyId,
                                int lifetime,
                                const QString &sdp,
                                bool answer,
                                QVector<std::pair<QString, QString>> msidToPurpose);

    QString partyId() const;
    QString sdp() const;
    // TODO make this a struct instead
    QJsonObject sdpStreamMetadata() const;
};
}
