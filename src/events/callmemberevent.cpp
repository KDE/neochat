// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "callmemberevent.h"

#include <QString>

using namespace Quotient;
using namespace Qt::Literals::StringLiterals;

CallMemberEventContent::CallMemberEventContent(const QJsonObject &json)
{
    for (const auto &membership : json["memberships"_L1].toArray()) {
        QList<Focus> foci;
        for (const auto &focus : membership["foci_active"_L1].toArray()) {
            foci.append(Focus{
                .livekitAlias = focus["livekit_alias"_L1].toString(),
                .livekitServiceUrl = focus["livekit_service_url"_L1].toString(),
                .type = focus["livekit"_L1].toString(),
            });
        }
        memberships.append(CallMembership{
            .application = membership["application"_L1].toString(),
            .callId = membership["call_id"_L1].toString(),
            .deviceId = membership["device_id"_L1].toString(),
            .expires = membership["expires"_L1].toInt(),
            .expiresTs = membership["expires"_L1].toVariant().value<uint64_t>(),
            .fociActive = foci,
            .membershipId = membership["membershipID"_L1].toString(),
            .scope = membership["scope"_L1].toString(),
        });
    }
}

QJsonObject CallMemberEventContent::toJson() const
{
    return {};
}
