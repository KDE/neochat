// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QPair>
#include <QQmlEngine>

#include <Quotient/events/roomevent.h>

#include "events/pollevent.h"
#include "models/pollanswermodel.h"

namespace Quotient
{
class PollResponseEvent;
}

class NeoChatRoom;

/**
 * @class PollHandler
 *
 * A class to help manage a poll in a room.
 *
 * A poll is made up of a start event that poses the question and possible answers,
 * and is followed by a series of response events as users in the room select
 * their choice. This purpose of the poll handler is to keep track of all this as
 * the poll is displayed as a single event in the timeline which merges all this
 * information.
 */
class PollHandler : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use NeoChatRoom::poll")

    /**
     * @brief The kind of the poll.
     */
    Q_PROPERTY(PollKind::Kind kind READ kind CONSTANT)

    /**
     * @brief The question for the poll.
     */
    Q_PROPERTY(QString question READ question NOTIFY questionChanged)

    /**
     * @brief Whether the poll has ended.
     */
    Q_PROPERTY(bool hasEnded READ hasEnded NOTIFY hasEndedChanged)

    /**
     * @brief The model to visualize the answers to this poll.
     */
    Q_PROPERTY(PollAnswerModel *answerModel READ answerModel CONSTANT)

    /**
     * @brief The total number of vote responses to the poll.
     */
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)

public:
    PollHandler() = default;
    PollHandler(NeoChatRoom *room, const QString &pollStartId);

    NeoChatRoom *room() const;

    PollKind::Kind kind() const;
    QString question() const;
    bool hasEnded() const;
    PollAnswerModel *answerModel();

    /**
     * @brief The total number of answer options.
     */
    int numAnswers() const;

    /**
     * @brief The answer at the given row.
     */
    Quotient::EventContent::Answer answerAtRow(int row) const;

    /**
     * @brief The number of responders who gave the answer ID.
     */
    int answerCountAtId(const QString &id) const;

    /**
     * @brief Check whether the given member has selected the given ID in their response.
     */
    bool checkMemberSelectedId(const QString &memberId, const QString &id) const;

    int totalCount() const;

    /**
     * @brief The current answer IDs with the most votes.
     */
    QStringList winningAnswerIds() const;

    /**
     * @brief Send an answer to the poll.
     */
    Q_INVOKABLE void sendPollAnswer(const QString &eventId, const QString &answerId);

Q_SIGNALS:
    void questionChanged();
    void hasEndedChanged();
    void answersChanged();
    void totalCountChanged();

    /**
     * @brief Emitted when the selected answers to the poll change.
     */
    void selectionsChanged();

private:
    QString m_pollStartId;

    void updatePoll(Quotient::RoomEventsRange events);

    void checkLoadRelations();
    void handleResponse(const Quotient::PollResponseEvent *event);
    QHash<QString, QDateTime> m_selectionTimestamps;
    QHash<QString, QStringList> m_selections;

    bool m_hasEnded = false;
    QDateTime m_endedTimestamp;

    QPointer<PollAnswerModel> m_answerModel;
};
