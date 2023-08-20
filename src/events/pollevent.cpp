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
    return contentJson()["org.matrix.msc3381.poll.start"_ls]["max_selections"_ls].toInt();
}

QString PollStartEvent::question() const
{
    return contentJson()["org.matrix.msc3381.poll.start"_ls]["question"_ls]["body"_ls].toString();
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
                          {{"org.matrix.msc3381.poll.response"_ls, QJsonObject{{"answers"_ls, QJsonArray::fromStringList(responses)}}},
                           {"m.relates_to"_ls, QJsonObject{{"rel_type"_ls, "m.reference"_ls}, {"event_id"_ls, pollStartEventId}}}}))
{
}
