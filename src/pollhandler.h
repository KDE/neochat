// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QPair>

class NeoChatRoom;

class PollHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)
    Q_PROPERTY(QString pollStartEventId READ pollStartEventId WRITE setPollStartEventId NOTIFY pollStartEventIdChanged)
    Q_PROPERTY(QJsonObject answers READ answers NOTIFY answersChanged)
    Q_PROPERTY(QJsonObject counts READ counts NOTIFY answersChanged)
    Q_PROPERTY(bool hasEnded READ hasEnded NOTIFY hasEndedChanged)
    Q_PROPERTY(int answerCount READ answerCount NOTIFY answersChanged)

public:
    PollHandler(QObject *parent = nullptr);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    QString pollStartEventId() const;
    void setPollStartEventId(const QString &eventId);

    bool hasEnded() const;
    int answerCount() const;

    void checkLoadRelations();

    QJsonObject answers() const;
    QJsonObject counts() const;
    Q_INVOKABLE void sendPollAnswer(const QString &eventId, const QString &answerId);

Q_SIGNALS:
    void roomChanged();
    void pollStartEventIdChanged();
    void answersChanged();
    void hasEndedChanged();

private:
    NeoChatRoom *m_room = nullptr;
    QString m_pollStartEventId;

    void handleAnswer(const QJsonObject &object, const QString &sender, QDateTime timestamp);
    QMap<QString, QDateTime> m_answerTimestamps;
    QJsonObject m_answers;
    int m_maxVotes = 1;
    bool m_hasEnded = false;
    QDateTime m_endedTimestamp;
};
