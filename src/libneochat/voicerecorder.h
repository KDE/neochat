// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include <QAudioInput>
#include <QBuffer>
#include <QMediaCaptureSession>
#include <QMediaFormat>
#include <QMediaRecorder>
#include <QPointer>

class VoiceRecorder : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QMediaRecorder *recorder READ recorder CONSTANT)
    // TODO: Remove once no longer required
    Q_PROPERTY(bool isSupported READ isSupported CONSTANT)

public:
    explicit VoiceRecorder(QObject *parent = nullptr);

    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();

    QMediaRecorder *recorder();

    bool isSupported() const;

private:
    QAudioInput m_input;
    QMediaCaptureSession m_session;
    QMediaRecorder m_recorder;
    QMediaFormat m_format;
};
