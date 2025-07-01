// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QTimer>

#include "neochatroom.h"
#include "neochatroommember.h"

class QAudioOutput;
class QMediaPlayer;

/**
 * @class CallManager
 *
 * Manages calls.
 */
class CallManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool isRinging READ isRinging NOTIFY isRingingChanged)
    Q_PROPERTY(NeoChatRoom *room READ room NOTIFY roomChanged)
    Q_PROPERTY(NeochatRoomMember *callingMember READ callingMember NOTIFY roomChanged)

public:
    static CallManager &instance()
    {
        static CallManager _instance;
        return _instance;
    }
    static CallManager *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    void ring(const QJsonObject &json, NeoChatRoom *room);
    void stopRinging();

    bool isRinging() const;

    void setCallsEnabled(bool enabled);

    NeoChatRoom *room() const;
    NeochatRoomMember *callingMember() const;

Q_SIGNALS:
    void isRingingChanged();
    void roomChanged();

private:
    CallManager();

    void ringUnchecked();
    bool m_ringing = false;
    QMediaPlayer *m_player;
    QAudioOutput *m_output;
    QTimer m_timer;
    bool m_callsEnabled = false;
    QPointer<NeoChatRoom> m_room;
    QString m_callingMember;
};
