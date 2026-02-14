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
    , m_buffer(new QBuffer)
    , m_format(QMediaFormat::FileFormat::Ogg)
{
    m_session.setAudioInput(&m_input);
    m_recorder.setAudioBitRate(24000);
    m_recorder.setAudioSampleRate(48000);
    m_format.setAudioCodec(QMediaFormat::AudioCodec::Opus);
    m_recorder.setAudioChannelCount(1);
    m_recorder.setMediaFormat(m_format);
    m_buffer->open(QIODevice::ReadWrite);
    m_recorder.setOutputDevice(m_buffer);
    m_session.setRecorder(&m_recorder);
}

VoiceRecorder::~VoiceRecorder()
{
    delete m_buffer;
}

void VoiceRecorder::startRecording()
{
    m_buffer->setData({});
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

void VoiceRecorder::send()
{
    Quotient::FileSourceInfo fileMetadata;
    QByteArray data;
    m_buffer->seek(0);

    if (m_room->usesEncryption()) {
        std::tie(fileMetadata, data) = Quotient::encryptFile(m_buffer->data());
        m_buffer->close();
        m_buffer->setData(data);
        m_buffer->open(QIODevice::ReadOnly);
    }

    auto room = m_room;
    auto buffer = m_buffer;
    auto duration = m_recorder.duration();
    m_buffer = nullptr;
    m_room->connection()->uploadContent(buffer, {}, u"audio/ogg"_s).then([fileMetadata, room, buffer, duration](const auto &job) mutable {
        QJsonObject mscFile{
            {u"mimetype"_s, u"audio/ogg"_s},
            {u"name"_s, u"Voice Message"_s},
            {u"size"_s, buffer->size()},
        };

        if (room->usesEncryption()) {
            mscFile[u"file"_s] = toJson(fileMetadata);
        } else {
            mscFile[u"url"_s] = job->contentUri().toString();
        }

        Quotient::setUrlInSourceInfo(fileMetadata, job->contentUri());
        QJsonObject content{
            {u"body"_s, u"Voice message"_s},
            {u"msgtype"_s, u"m.audio"_s},
            {u"org.matrix.msc1767.text"_s,
             QJsonObject{{u"body"_s, u"Voice Message (%1, %2)"_s.arg(KFormat().formatDuration(duration), KFormat().formatByteSize(buffer->size()))}}},
            {u"org.matrix.msc1767.file"_s, mscFile},
            {u"info"_s,
             QJsonObject{
                 {u"mimetype"_s, u"audio/ogg"_s},
                 {u"size"_s, buffer->size()},
                 {u"duration"_s, duration},
             }},
            {u"org.matrix.msc1767.audio"_s,
             QJsonObject{
                 {u"duration"_s, duration},
                 {u"waveform"_s, QJsonArray{}}, // TODO
             }},
            {u"org.matrix.msc3245.voice"_s, QJsonObject{}}};
        if (room->usesEncryption()) {
            content[u"file"_s] = toJson(fileMetadata);
        } else {
            content[u"url"_s] = job->contentUri().toString();
        }
        room->postJson(u"m.room.message"_s, content);
    });
}

void VoiceRecorder::setRoom(NeoChatRoom *room)
{
    m_room = room;
    Q_EMIT roomChanged();
}

NeoChatRoom *VoiceRecorder::room() const
{
    return m_room.get();
}

bool VoiceRecorder::isSupported() const
{
    return m_format.isSupported(QMediaFormat::Encode);
}
