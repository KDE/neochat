// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pollevent.h"
#include <Quotient/converters.h>

using namespace Quotient;

PollKind::Kind PollStartEvent::kind() const
{
    return content().kind;
}

int PollStartEvent::maxSelections() const
{
    return content().maxSelection > 0 ? content().maxSelection : 1;
}

QString PollStartEvent::question() const
{
    return content().question;
}

QList<EventContent::Answer> PollStartEvent::answers() const
{
    return content().answers;
}

PollResponseEvent::PollResponseEvent(const QJsonObject &obj)
    : RoomEvent(obj)
{
}

PollResponseEvent::PollResponseEvent(const QString &pollStartEventId, QStringList responses)
    : RoomEvent(basicJson(TypeId,
                          {{"org.matrix.msc3381.poll.response"_L1, QJsonObject{{"answers"_L1, QJsonArray::fromStringList(responses)}}},
                           {"m.relates_to"_L1, QJsonObject{{"rel_type"_L1, "m.reference"_L1}, {"event_id"_L1, pollStartEventId}}}}))
{
}

QStringList PollResponseEvent::selections() const
{
    const auto jsonSelections = contentPart<QJsonObject>("org.matrix.msc3381.poll.response"_L1)["answers"_L1].toArray();
    QStringList selections;
    for (const auto &selection : jsonSelections) {
        selections += selection.toString();
    }
    return selections;
}

std::optional<EventRelation> PollResponseEvent::relatesTo() const
{
    return contentPart<std::optional<EventRelation>>(RelatesToKey);
}

PollEndEvent::PollEndEvent(const QJsonObject &obj)
    : RoomEvent(obj)
{
}

PollEndEvent::PollEndEvent(const QString &pollStartEventId, const QString &endText)
    : RoomEvent(basicJson(TypeId,
                          {{"org.matrix.msc1767.text"_L1, endText},
                           {"org.matrix.msc3381.poll.end"_L1, QJsonObject{}},
                           {"m.relates_to"_L1, QJsonObject{{"rel_type"_L1, "m.reference"_L1}, {"event_id"_L1, pollStartEventId}}}}))
{
}

std::optional<EventRelation> PollEndEvent::relatesTo() const
{
    return contentPart<std::optional<EventRelation>>(RelatesToKey);
}
