// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pollevent.h"

using namespace Quotient;

PollStartEvent::PollStartEvent(const QJsonObject &obj)
    : RoomEvent(obj)
{
}

int PollStartEvent::maxSelections() const
{
    return contentJson()["org.matrix.msc3381.poll.start"_L1]["max_selections"_L1].toInt();
}

QString PollStartEvent::question() const
{
    return contentJson()["org.matrix.msc3381.poll.start"_L1]["question"_L1]["body"_L1].toString();
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
                          {{"org.matrix.msc3381.poll.response"_L1, QJsonObject{{"answers"_L1, QJsonArray::fromStringList(responses)}}},
                           {"m.relates_to"_L1, QJsonObject{{"rel_type"_L1, "m.reference"_L1}, {"event_id"_L1, pollStartEventId}}}}))
{
}
