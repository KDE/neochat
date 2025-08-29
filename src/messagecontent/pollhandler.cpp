// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pollhandler.h"

#include <KLocalization>

#include "events/pollevent.h"
#include "neochatroom.h"

#include <Quotient/csapi/relations.h>
#include <Quotient/events/roompowerlevelsevent.h>

#include <algorithm>
#include <qcontainerfwd.h>

using namespace Quotient;

PollHandler::PollHandler(NeoChatRoom *room, const QString &pollStartId)
    : QObject(nullptr)
    , m_pollStartId(pollStartId)
    , m_room(room)
{
    Q_ASSERT(room != nullptr);
    Q_ASSERT(!pollStartId.isEmpty());

    if (room != nullptr) {
        connect(room, &NeoChatRoom::aboutToAddNewMessages, this, &PollHandler::updatePoll);
        connect(room, &NeoChatRoom::pendingEventAboutToAdd, this, &PollHandler::handleEvent);
        checkLoadRelations();
    }
}

void PollHandler::updatePoll(Quotient::RoomEventsRange events)
{
    auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return;
    }
    for (const auto &event : events) {
        handleEvent(event.get());
    }
}

void PollHandler::checkLoadRelations()
{
    const auto pollStartEvent = m_room->getEvent(m_pollStartId).first;
    if (pollStartEvent == nullptr) {
        return;
    }

    m_room->connection()->callApi<GetRelatingEventsJob>(m_room->id(), pollStartEvent->id()).onResult([this](const auto &job) {
        for (const auto &event : job->chunk()) {
            handleEvent(event.get());
        }
    });
}

void PollHandler::handleEvent(Quotient::RoomEvent *event)
{
    auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return;
    }

    if (event->is<PollEndEvent>()) {
        const auto endEvent = eventCast<const PollEndEvent>(event);
        if (endEvent->relatesTo()->eventId != m_pollStartId) {
            return;
        }

        auto plEvent = m_room->currentState().get<RoomPowerLevelsEvent>();
        if (!plEvent) {
            return;
        }
        auto userPl = plEvent->powerLevelForUser(event->senderId());
        if (event->senderId() == pollStartEvent->senderId() || userPl >= plEvent->redact()) {
            m_hasEnded = true;
            m_endedTimestamp = event->originTimestamp();
            Q_EMIT hasEndedChanged();
        }
    }
    if (event->is<PollResponseEvent>()) {
        handleResponse(eventCast<const PollResponseEvent>(event));
    }
    if (event->contentPart<QJsonObject>("m.relates_to"_L1).contains("rel_type"_L1)
        && event->contentPart<QJsonObject>("m.relates_to"_L1)["rel_type"_L1].toString() == "m.replace"_L1
        && event->contentPart<QJsonObject>("m.relates_to"_L1)["event_id"_L1].toString() == pollStartEvent->id()) {
        Q_EMIT questionChanged();
        Q_EMIT answersChanged();
    }
}

void PollHandler::handleResponse(const Quotient::PollResponseEvent *event)
{
    if (event == nullptr) {
        return;
    }

    if (event->relatesTo()->eventId != m_pollStartId) {
        return;
    }

    // If there is no origin timestamp it's pending and therefore must be newer.
    if ((event->originTimestamp() > m_selectionTimestamps[event->senderId()] || event->id().isEmpty())
        && (!m_hasEnded || event->originTimestamp() < m_endedTimestamp)) {
        m_selectionTimestamps[event->senderId()] = event->originTimestamp();

        const auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
        if (pollStartEvent == nullptr) {
            return;
        }

        m_selections[event->senderId()] = event->selections().size() > 0 ? event->selections().first(pollStartEvent->maxSelections()) : event->selections();
        if (m_selections.contains(event->senderId()) && m_selections[event->senderId()].isEmpty()) {
            m_selections.remove(event->senderId());
        }
    }

    Q_EMIT totalCountChanged();
    Q_EMIT selectionsChanged();
}

NeoChatRoom *PollHandler::room() const
{
    return m_room.get();
}

QString PollHandler::question() const
{
    if (!m_room) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return {};
    }
    return pollStartEvent->question();
}

int PollHandler::numAnswers() const
{
    if (m_room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return {};
    }
    return pollStartEvent->answers().length();
}

Quotient::EventContent::Answer PollHandler::answerAtRow(int row) const
{
    if (m_room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return {};
    }
    return pollStartEvent->answers()[row];
}

int PollHandler::answerCountAtId(const QString &id) const
{
    int count = 0;
    for (const auto &selection : m_selections) {
        if (selection.contains(id)) {
            count++;
        }
    }
    return count;
}

bool PollHandler::checkMemberSelectedId(const QString &memberId, const QString &id) const
{
    return m_selections[memberId].contains(id);
}

PollKind::Kind PollHandler::kind() const
{
    if (m_room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return {};
    }
    return pollStartEvent->kind();
}

PollAnswerModel *PollHandler::answerModel()
{
    if (m_answerModel == nullptr) {
        m_answerModel = new PollAnswerModel(this);
    }
    return m_answerModel;
}

int PollHandler::totalCount() const
{
    int votes = 0;
    for (const auto &selection : m_selections) {
        votes += selection.size();
    }
    return votes;
}

QStringList PollHandler::winningAnswerIds() const
{
    if (m_room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return {};
    }

    QStringList currentWinners;
    for (const auto &answer : pollStartEvent->answers()) {
        if (currentWinners.isEmpty()) {
            currentWinners += answer.id;
            continue;
        }
        if (answerCountAtId(currentWinners.first()) < answerCountAtId(answer.id)) {
            currentWinners.clear();
            currentWinners += answer.id;
            continue;
        }
        if (answerCountAtId(currentWinners.first()) == answerCountAtId(answer.id)) {
            currentWinners += answer.id;
        }
    }
    return currentWinners;
}

void PollHandler::sendPollAnswer(const QString &eventId, const QString &answerId)
{
    Q_ASSERT(eventId.length() > 0);
    Q_ASSERT(answerId.length() > 0);
    if (m_room == nullptr) {
        qWarning() << "PollHandler is empty, cannot send an answer.";
        return;
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return;
    }

    QStringList ownAnswers = m_selections[m_room->localMember().id()];
    if (ownAnswers.contains(answerId)) {
        ownAnswers.erase(std::remove_if(ownAnswers.begin(), ownAnswers.end(), [answerId](const auto &it) {
            return answerId == it;
        }));
    } else {
        while (ownAnswers.size() >= pollStartEvent->maxSelections() && ownAnswers.size() > 0) {
            ownAnswers.pop_front();
        }
        ownAnswers.insert(0, answerId);
    }

    const auto &response = m_room->post<PollResponseEvent>(eventId, ownAnswers);
    handleResponse(eventCast<const PollResponseEvent>(response.event()));
}

bool PollHandler::hasEnded() const
{
    return m_hasEnded;
}

void PollHandler::endPoll() const
{
    room()->post<PollEndEvent>(m_pollStartId, endText());
}

QString PollHandler::endText() const
{
    if (m_room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(m_room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return {};
    }
    int maxCount = 0;
    QString answerText = {};
    for (const auto &answer : pollStartEvent->answers()) {
        const auto currentCount = answerCountAtId(answer.id);
        if (currentCount > maxCount) {
            maxCount = currentCount;
            answerText = answer.text;
        }
    }

    return i18nc("%1 is the poll answer that had the most votes", "The poll has ended. Top answer: %1", answerText);
}

#include "moc_pollhandler.cpp"
