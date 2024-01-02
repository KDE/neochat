// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pollhandler.h"

#include "neochatroom.h"

#include <Quotient/csapi/relations.h>
#include <Quotient/events/roompowerlevelsevent.h>

#include <algorithm>

using namespace Quotient;

PollHandler::PollHandler(NeoChatRoom *room, const Quotient::PollStartEvent *pollStartEvent)
    : QObject(room)
    , m_pollStartEvent(pollStartEvent)
{
    if (room != nullptr && m_pollStartEvent != nullptr) {
        connect(room, &NeoChatRoom::aboutToAddNewMessages, this, &PollHandler::updatePoll);
        checkLoadRelations();
    }
}

void PollHandler::updatePoll(Quotient::RoomEventsRange events)
{
    // This function will never be called if the PollHandler was not initialized with
    // a NeoChatRoom as parent and a PollStartEvent so no need to null check.
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    for (const auto &event : events) {
        if (event->is<PollEndEvent>()) {
            auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
            if (!plEvent) {
                continue;
            }
            auto userPl = plEvent->powerLevelForUser(event->senderId());
            if (event->senderId() == m_pollStartEvent->senderId() || userPl >= plEvent->redact()) {
                m_hasEnded = true;
                m_endedTimestamp = event->originTimestamp();
                Q_EMIT hasEndedChanged();
            }
        }
        if (event->is<PollResponseEvent>()) {
            handleAnswer(event->contentJson(), event->senderId(), event->originTimestamp());
        }
        if (event->contentPart<QJsonObject>("m.relates_to"_ls).contains("rel_type"_ls)
            && event->contentPart<QJsonObject>("m.relates_to"_ls)["rel_type"_ls].toString() == "m.replace"_ls
            && event->contentPart<QJsonObject>("m.relates_to"_ls)["event_id"_ls].toString() == m_pollStartEvent->id()) {
            Q_EMIT questionChanged();
            Q_EMIT optionsChanged();
        }
    }
}

void PollHandler::checkLoadRelations()
{
    // This function will never be called if the PollHandler was not initialized with
    // a NeoChatRoom as parent and a PollStartEvent so no need to null check.
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    m_maxVotes = m_pollStartEvent->maxSelections();
    auto job = room->connection()->callApi<GetRelatingEventsJob>(room->id(), m_pollStartEvent->id());
    connect(job, &BaseJob::success, this, [this, job, room]() {
        for (const auto &event : job->chunk()) {
            if (event->is<PollEndEvent>()) {
                auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
                if (!plEvent) {
                    continue;
                }
                auto userPl = plEvent->powerLevelForUser(event->senderId());
                if (event->senderId() == m_pollStartEvent->senderId() || userPl >= plEvent->redact()) {
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
        for (const auto &answer : content["org.matrix.msc3381.poll.response"_ls]["answers"_ls].toArray()) {
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

QString PollHandler::question() const
{
    if (m_pollStartEvent == nullptr) {
        return {};
    }
    return m_pollStartEvent->contentPart<QJsonObject>("org.matrix.msc3381.poll.start"_ls)["question"_ls].toObject()["body"_ls].toString();
}

QJsonArray PollHandler::options() const
{
    if (m_pollStartEvent == nullptr) {
        return {};
    }
    return m_pollStartEvent->contentPart<QJsonObject>("org.matrix.msc3381.poll.start"_ls)["answers"_ls].toArray();
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

QString PollHandler::kind() const
{
    if (m_pollStartEvent == nullptr) {
        return {};
    }
    return m_pollStartEvent->contentPart<QJsonObject>("org.matrix.msc3381.poll.start"_ls)["kind"_ls].toString();
}

void PollHandler::sendPollAnswer(const QString &eventId, const QString &answerId)
{
    Q_ASSERT(eventId.length() > 0);
    Q_ASSERT(answerId.length() > 0);
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        qWarning() << "PollHandler is empty, cannot send an answer.";
        return;
    }
    QStringList ownAnswers;
    for (const auto &answer : m_answers[room->localUser()->id()].toArray()) {
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
    handleAnswer(response->contentJson(), room->localUser()->id(), QDateTime::currentDateTime());
    room->postEvent(response);
}

bool PollHandler::hasEnded() const
{
    return m_hasEnded;
}

int PollHandler::answerCount() const
{
    return m_answers.size();
}

#include "moc_pollhandler.cpp"
