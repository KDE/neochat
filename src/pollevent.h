// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <events/eventcontent.h>
#include <events/roomevent.h>

namespace Quotient
{
class PollStartEvent : public RoomEvent
{
public:
    QUO_EVENT(PollStartEvent, "org.matrix.msc3381.poll.start");
    explicit PollStartEvent(const QJsonObject &obj);

    int maxSelections() const;
    QString question() const;
};

class PollResponseEvent : public RoomEvent
{
public:
    QUO_EVENT(PollResponseEvent, "org.matrix.msc3381.poll.response");
    explicit PollResponseEvent(const QJsonObject &obj);
    explicit PollResponseEvent(const QString &pollStartEventId, QStringList responses);
};

class PollEndEvent : public RoomEvent
{
public:
    QUO_EVENT(PollEndEvent, "org.matrix.msc3381.poll.end");
    explicit PollEndEvent(const QJsonObject &obj);
};
}
