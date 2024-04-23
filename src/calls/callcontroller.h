// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QVideoSink>

#include "events/callmemberevent.h"

#include "room.qpb.h"

namespace livekit::proto
{
class FfiEvent;
class ConnectCallback;
class DisposeCallback;
class RoomEvent;
}

class LivekitMediaPlayer;

class NeoChatRoom;
class QAudioSink;

class CallController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static CallController &instance()
    {
        static CallController _instance;
        return _instance;
    }

    static CallController *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    void handleCallMemberEvent(const Quotient::CallMemberEvent *event, NeoChatRoom *room);

    // Internal. Do not use.
    void handleEvent(livekit::proto::FfiEvent &&event);
    Q_INVOKABLE void setVideoSink(QObject *sink);
    void setCameraVideoSink(QVideoSink *videoSink);

    Q_INVOKABLE void toggleCamera();

Q_SIGNALS:
    void callStarted();

private:
    CallController();
    void init();

    QMap<uint64_t, QPointer<NeoChatRoom>> m_connectingRooms;
    std::map<uint64_t, livekit::proto::OwnedRoom> m_rooms;
    void handleConnect(livekit::proto::ConnectCallback &&callback);
    void handleDispose(livekit::proto::DisposeCallback &&callback);
    void handleRoomEvent(livekit::proto::RoomEvent &&event);
    void publishTrack(uint64_t id);

    QIODevice *audioData = nullptr;
    QAudioSink *sink;
    QVideoSink *m_sink;
    uint64_t resampler;
    QVideoSink *m_cameraVideoSink = nullptr;
    uint64_t localParticipant = 100000;
    QString m_localVideoTrackSid;
    uint64_t m_localVideoTrackId;
};

class LivekitVideoSink : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    Q_PROPERTY(QVideoSink *videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged REQUIRED)
    using QObject::QObject;

    void setVideoSink(QVideoSink *videoSink);
    QVideoSink *videoSink() const;

Q_SIGNALS:
    void videoSinkChanged();

private:
    QVideoSink *m_videoSink = nullptr;
};
