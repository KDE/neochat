// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <Quotient/events/roomevent.h>

namespace Quotient
{
/**
 * @class PollStartEvent
 *
 * Class to define a poll start event.
 *
 * See MSC3381 for full details on polls in the matrix spec
 * https://github.com/matrix-org/matrix-spec-proposals/blob/travis/msc/polls/proposals/3381-polls.md.
 *
 * @sa Quotient::RoomEvent
 */
class PollStartEvent : public RoomEvent
{
public:
    QUO_EVENT(PollStartEvent, "org.matrix.msc3381.poll.start");
    explicit PollStartEvent(const QJsonObject &obj);

    /**
     * @brief The maximum number of options a user can select in a poll.
     */
    int maxSelections() const;

    /**
     * @brief The question being asked in the poll.
     */
    QString question() const;
};

/**
 * @class PollResponseEvent
 *
 * Class to define a poll response event.
 *
 * See MSC3381 for full details on polls in the matrix spec
 * https://github.com/matrix-org/matrix-spec-proposals/blob/travis/msc/polls/proposals/3381-polls.md.
 *
 * @sa Quotient::RoomEvent
 */
class PollResponseEvent : public RoomEvent
{
public:
    QUO_EVENT(PollResponseEvent, "org.matrix.msc3381.poll.response");
    explicit PollResponseEvent(const QJsonObject &obj);
    explicit PollResponseEvent(const QString &pollStartEventId, QStringList responses);
};

/**
 * @class PollEndEvent
 *
 * Class to define a poll end event.
 *
 * See MSC3381 for full details on polls in the matrix spec
 * https://github.com/matrix-org/matrix-spec-proposals/blob/travis/msc/polls/proposals/3381-polls.md.
 *
 * @sa Quotient::RoomEvent
 */
class PollEndEvent : public RoomEvent
{
public:
    QUO_EVENT(PollEndEvent, "org.matrix.msc3381.poll.end");
    explicit PollEndEvent(const QJsonObject &obj);
};
}
