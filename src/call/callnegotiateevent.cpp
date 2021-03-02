// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "callnegotiateevent.h"

using namespace Quotient;

CallNegotiateEvent::CallNegotiateEvent(const QString &callId,
                                       const QString &partyId,
                                       int lifetime,
                                       const QString &sdp,
                                       bool answer,
                                       QVector<std::pair<QString, QString>> msidToPurpose)
    : EventTemplate(callId,
                    {
                        {QStringLiteral("lifetime"), lifetime},
                        {QStringLiteral("version"), 1},
                        {QStringLiteral("description"),
                         QJsonObject{{QStringLiteral("type"), answer ? QStringLiteral("answer") : QStringLiteral("offer")}, {QStringLiteral("sdp"), sdp}}},
                        {QStringLiteral("party_id"), partyId},
                    })
{
    QJsonObject metadata;
    for (const auto &[stream, purpose] : msidToPurpose) {
        QJsonObject data = {{"purpose", purpose}};
        metadata[stream] = purpose;
    }
    auto content = editJson();
    content["org.matrix.msc3077.sdp_stream_metadata"] = metadata;
    editJson()["content"] = content;
}

CallNegotiateEvent::CallNegotiateEvent(const QJsonObject &json)
    : EventTemplate(json)
{
}

QString CallNegotiateEvent::partyId() const
{
    return contentJson()["party_id"].toString();
}

QString CallNegotiateEvent::sdp() const
{
    return contentJson()["description"]["sdp"].toString();
}

QJsonObject CallNegotiateEvent::sdpStreamMetadata() const
{
    return contentJson()["org.matrix.msc3077.sdp_stream_metadata"].toObject();
}
