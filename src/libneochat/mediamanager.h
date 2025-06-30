// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QTimer>

class NeoChatRoom;
class QAudioOutput;
class QMediaPlayer;

/**
 * @class MediaManager
 *
 * Manages media playback, like voice/audio messages, videos, etc.
 */
class MediaManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool isRinging READ isRinging NOTIFY isRingingChanged)

public:
    static MediaManager &instance()
    {
        static MediaManager _instance;
        return _instance;
    }
    static MediaManager *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    /**
     * @brief Notify other objects that media playback has started.
     */
    Q_INVOKABLE void startPlayback();

    void ring(const QJsonObject &json, NeoChatRoom *room);
    void stopRinging();

    bool isRinging() const;

Q_SIGNALS:
    /**
     * @brief Emitted when any media player starts playing. Other objects should stop / pause playback.
     */
    void playbackStarted();
    void isRingingChanged();

private:
    MediaManager();

    void ringUnchecked();
    bool m_ringing = false;
    QMediaPlayer *m_player;
    QAudioOutput *m_output;
    QTimer m_timer;
};
