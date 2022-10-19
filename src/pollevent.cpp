// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pollevent.h"

using namespace Quotient;

PollStartEvent::PollStartEvent(const QJsonObject &obj)
    : RoomEvent(obj)
{
}

int PollStartEvent::maxSelections() const
{
    return contentJson()["org.matrix.msc3381.poll.start"]["max_selections"].toInt();
}

QString PollStartEvent::question() const
{
    return contentJson()["org.matrix.msc3381.poll.start"]["question"]["body"].toString();
}

PollResponseEvent::PollResponseEvent(const QJsonObject &obj)
    : RoomEvent(obj)
{
}

PollEndEvent::PollEndEvent(const QJsonObject &obj)
    : RoomEvent(obj)
{
}

PollResponseEvent::PollResponseEvent(const QString &pollStartEventId, QStringList responses)
    : RoomEvent(basicJson(TypeId,
                          {{"org.matrix.msc3381.poll.response", QJsonObject{{"answers", QJsonArray::fromStringList(responses)}}},
                           {"m.relates_to", QJsonObject{{"rel_type", "m.reference"}, {"event_id", pollStartEventId}}}}))
{
}
