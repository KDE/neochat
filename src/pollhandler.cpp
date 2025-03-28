// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pollhandler.h"

#include "events/pollevent.h"
#include "neochatroom.h"
#include "pollanswermodel.h"

#include <Quotient/csapi/relations.h>
#include <Quotient/events/roompowerlevelsevent.h>

#include <algorithm>
#include <qcontainerfwd.h>

using namespace Quotient;

PollHandler::PollHandler(NeoChatRoom *room, const QString &pollStartId)
    : QObject(room)
    , m_pollStartId(pollStartId)
{
    Q_ASSERT(room != nullptr);
    Q_ASSERT(!pollStartId.isEmpty());

    if (room != nullptr) {
        connect(room, &NeoChatRoom::aboutToAddNewMessages, this, &PollHandler::updatePoll);
        checkLoadRelations();
    }
}

void PollHandler::updatePoll(Quotient::RoomEventsRange events)
{
    // This function will never be called if the PollHandler was not initialized with
    // a NeoChatRoom as parent and a PollStartEvent so no need to null check.
    const auto room = dynamic_cast<NeoChatRoom *>(parent());
    auto pollStartEvent = eventCast<const PollStartEvent>(room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return;
    }

    for (const auto &event : events) {
        if (event->is<PollEndEvent>()) {
            const auto endEvent = eventCast<const PollEndEvent>(event);
            if (endEvent->relatesTo()->eventId != m_pollStartId) {
                continue;
            }

            auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
            if (!plEvent) {
                continue;
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
}

void PollHandler::checkLoadRelations()
{
    // This function will never be called if the PollHandler was not initialized with
    // a NeoChatRoom as parent and a PollStartEvent so no need to null check.
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    const auto pollStartEvent = room->getEvent(m_pollStartId).first;
    if (pollStartEvent == nullptr) {
        return;
    }

    auto job = room->connection()->callApi<GetRelatingEventsJob>(room->id(), pollStartEvent->id());
    connect(job, &BaseJob::success, this, [this, job, room, pollStartEvent]() {
        for (const auto &event : job->chunk()) {
            if (event->is<PollEndEvent>()) {
                const auto endEvent = eventCast<const PollEndEvent>(event);
                if (endEvent->relatesTo()->eventId != m_pollStartId) {
                    continue;
                }

                auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
                if (!plEvent) {
                    continue;
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
        }
    });
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

        // This function will never be called if the PollHandler was not initialized with
        // a NeoChatRoom as parent and a PollStartEvent so no need to null check.
        auto room = dynamic_cast<NeoChatRoom *>(parent());
        const auto pollStartEvent = eventCast<const PollStartEvent>(room->getEvent(m_pollStartId).first);
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
    return dynamic_cast<NeoChatRoom *>(parent());
}

QString PollHandler::question() const
{
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return {};
    }
    return pollStartEvent->question();
}

int PollHandler::numAnswers() const
{
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return {};
    }
    return pollStartEvent->answers().length();
}

Quotient::EventContent::Answer PollHandler::answerAtRow(int row) const
{
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(room->getEvent(m_pollStartId).first);
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
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(room->getEvent(m_pollStartId).first);
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
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        return {};
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(room->getEvent(m_pollStartId).first);
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
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        qWarning() << "PollHandler is empty, cannot send an answer.";
        return;
    }
    auto pollStartEvent = eventCast<const PollStartEvent>(room->getEvent(m_pollStartId).first);
    if (pollStartEvent == nullptr) {
        return;
    }

    QStringList ownAnswers = m_selections[room->localMember().id()];
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

    const auto &response = room->post<PollResponseEvent>(eventId, ownAnswers);
    handleResponse(eventCast<const PollResponseEvent>(response.event()));
}

bool PollHandler::hasEnded() const
{
    return m_hasEnded;
}

#include "moc_pollhandler.cpp"
