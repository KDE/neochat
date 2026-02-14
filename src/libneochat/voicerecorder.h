// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "neochatroom.h"
#include <QAudioInput>
#include <QBuffer>
#include <QMediaCaptureSession>
#include <QMediaFormat>
#include <QMediaRecorder>

class VoiceRecorder : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QMediaRecorder *recorder READ recorder CONSTANT)
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged REQUIRED)
    // TODO: Remove once no longer required
    Q_PROPERTY(bool isSupported READ isSupported CONSTANT)

public:
    explicit VoiceRecorder(QObject *parent = nullptr);
    ~VoiceRecorder() override;

    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void send();

    QMediaRecorder *recorder();

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    bool isSupported() const;

Q_SIGNALS:
    void roomChanged();

private:
    QAudioInput m_input;
    QMediaCaptureSession m_session;
    QMediaRecorder m_recorder;
    QBuffer *m_buffer;
    QPointer<NeoChatRoom> m_room;
    QMediaFormat m_format;
};
