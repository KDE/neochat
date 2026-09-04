// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "voicerecorder.h"

#include <QFile>
#include <QTemporaryFile>

#include <KFormat>

#include <Quotient/events/filesourceinfo.h>

using namespace Qt::Literals::StringLiterals;

VoiceRecorder::VoiceRecorder(QObject *parent)
    : QObject(parent)
    , m_format(QMediaFormat::FileFormat::Ogg)
{
    m_session.setAudioInput(&m_input);
    m_recorder.setAudioBitRate(24000);
    m_recorder.setAudioSampleRate(48000);
    m_format.setAudioCodec(QMediaFormat::AudioCodec::Opus);
    m_recorder.setAudioChannelCount(1);
    m_recorder.setMediaFormat(m_format);
    m_session.setRecorder(&m_recorder);
}

void VoiceRecorder::startRecording()
{
    if (!m_recorder.outputDevice()) {
        auto buffer = new QBuffer();
        buffer->open(QIODevice::ReadWrite);
        m_recorder.setOutputDevice(buffer);
    }
    m_recorder.record();
}

void VoiceRecorder::stopRecording()
{
    m_recorder.stop();
}

QMediaRecorder *VoiceRecorder::recorder()
{
    return &m_recorder;
}

bool VoiceRecorder::isSupported() const
{
    return m_format.isSupported(QMediaFormat::Encode);
}

#include "moc_voicerecorder.cpp"
