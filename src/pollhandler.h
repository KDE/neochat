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

    /**
     * @brief The question for the poll.
     */
    Q_PROPERTY(QString question READ question NOTIFY questionChanged)

    /**
     * @brief The list of possible answers to the poll.
     */
    Q_PROPERTY(QJsonArray options READ options NOTIFY optionsChanged)

    /**
     * @brief The list of answer responses to the poll from users in the room.
     */
    Q_PROPERTY(QJsonObject answers READ answers NOTIFY answersChanged)

    /**
     * @brief The list number of votes for each answer in the poll.
     */
    Q_PROPERTY(QJsonObject counts READ counts NOTIFY answersChanged)

    /**
     * @brief Whether the poll has ended.
     */
    Q_PROPERTY(bool hasEnded READ hasEnded NOTIFY hasEndedChanged)

    /**
     * @brief The total number of answers to the poll.
     */
    Q_PROPERTY(int answerCount READ answerCount NOTIFY answersChanged)

    /**
     * @brief The kind of the poll.
     */
    Q_PROPERTY(QString kind READ kind CONSTANT)

public:
    PollHandler() = default;
    PollHandler(NeoChatRoom *room, const Quotient::PollStartEvent *pollStartEvent);

    bool hasEnded() const;
    int answerCount() const;

    QString question() const;
    QJsonArray options() const;
    QJsonObject answers() const;
    QJsonObject counts() const;
    QString kind() const;

    /**
     * @brief Send an answer to the poll.
     */
    Q_INVOKABLE void sendPollAnswer(const QString &eventId, const QString &answerId);

Q_SIGNALS:
    void questionChanged();
    void optionsChanged();
    void answersChanged();
    void hasEndedChanged();

private:
    const Quotient::PollStartEvent *m_pollStartEvent;

    void updatePoll(Quotient::RoomEventsRange events);

    void checkLoadRelations();
    void handleAnswer(const QJsonObject &object, const QString &sender, QDateTime timestamp);
    QMap<QString, QDateTime> m_answerTimestamps;
    QJsonObject m_answers;
    int m_maxVotes = 1;
    bool m_hasEnded = false;
    QDateTime m_endedTimestamp;
};
