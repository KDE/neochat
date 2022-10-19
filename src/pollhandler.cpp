// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pollhandler.h"
#include "neochatroom.h"
#include "pollevent.h"
#include <algorithm>
#include <csapi/relations.h>
#include <events/roompowerlevelsevent.h>
#include <user.h>

using namespace Quotient;

PollHandler::PollHandler(QObject *parent)
    : QObject(parent)
{
    connect(this, &PollHandler::roomChanged, this, &PollHandler::checkLoadRelations);
    connect(this, &PollHandler::pollStartEventIdChanged, this, &PollHandler::checkLoadRelations);
}

NeoChatRoom *PollHandler::room() const
{
    return m_room;
}

void PollHandler::setRoom(NeoChatRoom *room)
{
    if (m_room == room) {
        return;
    }
    if (m_room) {
        disconnect(m_room, nullptr, this, nullptr);
    }
    connect(room, &NeoChatRoom::aboutToAddNewMessages, this, [this](Quotient::RoomEventsRange events) {
        for (const auto &event : events) {
            if (event->is<PollEndEvent>()) {
                auto pl = m_room->getCurrentState<RoomPowerLevelsEvent>();
                auto userPl = pl->powerLevelForUser(event->senderId());
                if (event->senderId() == (*m_room->findInTimeline(m_pollStartEventId))->senderId() || userPl >= pl->redact()) {
                    m_hasEnded = true;
                    m_endedTimestamp = event->originTimestamp();
                    Q_EMIT hasEndedChanged();
                }
            }
            if (event->is<PollResponseEvent>()) {
                handleAnswer(event->contentJson(), event->senderId(), event->originTimestamp());
            }
        }
    });
    m_room = room;
    Q_EMIT roomChanged();
}

QString PollHandler::pollStartEventId() const
{
    return m_pollStartEventId;
}

void PollHandler::setPollStartEventId(const QString &eventId)
{
    if (eventId == m_pollStartEventId) {
        return;
    }
    m_pollStartEventId = eventId;
    Q_EMIT pollStartEventIdChanged();
}

void PollHandler::checkLoadRelations()
{
    if (!m_room || m_pollStartEventId.isEmpty()) {
        return;
    }
    m_maxVotes = eventCast<const PollStartEvent>(&**m_room->findInTimeline(m_pollStartEventId))->maxSelections();
    auto job = m_room->connection()->callApi<GetRelatingEventsJob>(m_room->id(), m_pollStartEventId);
    connect(job, &BaseJob::success, this, [this, job]() {
        for (const auto &event : job->chunk()) {
            if (event->is<PollEndEvent>()) {
                auto pl = m_room->getCurrentState<RoomPowerLevelsEvent>();
                auto userPl = pl->powerLevelForUser(event->senderId());
                if (event->senderId() == (*m_room->findInTimeline(m_pollStartEventId))->senderId() || userPl >= pl->redact()) {
                    m_hasEnded = true;
                    m_endedTimestamp = event->originTimestamp();
                    Q_EMIT hasEndedChanged();
                }
            }
            if (event->is<PollResponseEvent>()) {
                handleAnswer(event->contentJson(), event->senderId(), event->originTimestamp());
            }
        }
    });
}

void PollHandler::handleAnswer(const QJsonObject &content, const QString &sender, QDateTime timestamp)
{
    if (timestamp > m_answerTimestamps[sender] && (!m_hasEnded || timestamp < m_endedTimestamp)) {
        m_answerTimestamps[sender] = timestamp;
        m_answers[sender] = {};
        int i = 0;
        for (const auto &answer : content["org.matrix.msc3381.poll.response"]["answers"].toArray()) {
            auto array = m_answers[sender].toArray();
            array.insert(0, answer);
            m_answers[sender] = array;
            i++;
            if (i == m_maxVotes) {
                break;
            }
        }
        for (const auto &key : m_answers.keys()) {
            if (m_answers[key].toArray().isEmpty()) {
                m_answers.remove(key);
            }
        }
    }
    Q_EMIT answersChanged();
}

QJsonObject PollHandler::answers() const
{
    return m_answers;
}

QJsonObject PollHandler::counts() const
{
    QJsonObject counts;
    for (const auto &answer : m_answers) {
        for (const auto &id : answer.toArray()) {
            counts[id.toString()] = counts[id.toString()].toInt() + 1;
        }
    }
    return counts;
}

void PollHandler::sendPollAnswer(const QString &eventId, const QString &answerId)
{
    Q_ASSERT(eventId.length() > 0);
    Q_ASSERT(answerId.length() > 0);
    QStringList ownAnswers;
    for (const auto &answer : m_answers[m_room->localUser()->id()].toArray()) {
        ownAnswers += answer.toString();
    }
    if (ownAnswers.contains(answerId)) {
        ownAnswers.erase(std::remove_if(ownAnswers.begin(), ownAnswers.end(), [answerId](const auto &it) {
            return answerId == it;
        }));
    } else {
        while (ownAnswers.size() >= m_maxVotes) {
            ownAnswers.pop_front();
        }
        ownAnswers.insert(0, answerId);
    }

    auto response = new PollResponseEvent(eventId, ownAnswers);
    handleAnswer(response->contentJson(), m_room->localUser()->id(), QDateTime::currentDateTime());
    m_room->postEvent(response);
}

bool PollHandler::hasEnded() const
{
    return m_hasEnded;
}

int PollHandler::answerCount() const
{
    return m_answers.size();
}
