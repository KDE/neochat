// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QObject>
#include <QQmlEngine>
#include <QTimer>

class NeoChatRoom;

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

    /**
     * Starts ringing if the criteria (see MSC / spec) are met.
     */
    void ring(const QJsonObject &json, NeoChatRoom *room);

    bool isRinging() const;

Q_SIGNALS:
    /**
     * @brief Emitted when any media player starts playing. Other objects should stop / pause playback.
     */
    void playbackStarted();

    void showIncomingCallDialog();
    void closeIncomingCallDialog();

private:
    void ringUnchecked();
    void stopRinging();
    QMediaPlayer *m_player;
    QAudioOutput *m_output;
    QTimer *m_timer;
    bool m_ringing = false;

    explicit MediaManager(QObject *parent = nullptr);
};
