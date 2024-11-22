// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "callcontroller.h"

#include <QAudioSink>
#include <QMediaDevices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProtobufSerializer>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoSink>

#include <livekit_ffi.h>

#include <Quotient/csapi/openid.h>

#include "audio_frame.qpb.h"
#include "ffi.qpb.h"

#include "livekitlogmodel.h"
#include "neochatroom.h"
#include "track.qpb.h"
#include "video_frame.qpb.h"

using namespace livekit::proto;
using namespace Quotient;

extern "C" {
void livekit_ffi_initialize(void(ffiCallbackFn(const uint8_t *, size_t)), bool capture_logs, const char *, const char *);
}

void callback(const uint8_t *data, size_t length)
{
    auto byteArrayData = QByteArray::fromRawData((const char *)data, length);
    QProtobufSerializer serializer;
    FfiEvent event;
    event.deserialize(&serializer, byteArrayData);
    CallController::instance().handleEvent(std::move(event));
}

CallController::CallController()
    : QObject()
{
    init();
}

void CallController::init()
{
    qRegisterProtobufTypes();
    livekit_ffi_initialize(callback, true, "test", "1.0");
}

static void handleLog(LogRecordRepeated &&logs)
{
    for (const auto &log : logs) {
        if (log.level() < 3) {
            qWarning() << log.message();
        }
    }
    LivekitLogModel::instance().addMessages(logs);
}

void CallController::handleConnect(ConnectCallback &&callback)
{
    qWarning() << "Connecting to" << callback.result().room().info().name() << "with id" << callback.asyncId();
    if (!m_connectingRooms.contains(callback.asyncId()) || !m_connectingRooms[callback.asyncId()]
        || m_connectingRooms[callback.asyncId()]->id() != callback.result().room().info().name()) {
        qWarning() << "Connecting to unexpected room";
        return;
    }
    m_connectingRooms.remove(callback.asyncId());
    m_rooms[callback.asyncId()] = callback.result().room();
    localParticipant = callback.result().localParticipant().handle().id_proto();
    Q_EMIT connected();
}

void CallController::handleDispose(DisposeCallback &&callback)
{
    qWarning() << "Disposing" << callback.asyncId();
    if (m_rooms.contains(callback.asyncId())) {
        qWarning() << "  room" << m_rooms[callback.asyncId()].info().name();
        m_rooms.erase(callback.asyncId());
    } else {
        qWarning() << "  unknown object";
    }
}

void CallController::handleRoomEvent(livekit::proto::RoomEvent &&event)
{
    if (event.hasParticipantConnected()) {
        qWarning() << "Participant connected" << event.participantConnected().info().info().identity();
    } else if (event.hasParticipantDisconnected()) {
        qWarning() << "Participant connected" << event.participantDisconnected().participantIdentity();
    } else if (event.hasLocalTrackPublished()) {
        qWarning() << "Local track published";
        m_localVideoTrackSid = event.localTrackPublished().trackSid();
    } else if (event.hasLocalTrackUnpublished()) {
        qWarning() << "Local track unpublished";
    } else if (event.hasTrackPublished()) {
        qWarning() << "Track published";
    } else if (event.hasTrackUnpublished()) {
        qWarning() << "Track unpublished";
    } else if (event.hasTrackSubscribed()) {
        qWarning() << "Track subscribed";

        auto track = event.trackSubscribed().track();
        if (track.info().kind() == TrackKindGadget::KIND_AUDIO) {
            NewAudioStreamRequest audioStreamRequest;
            audioStreamRequest.setTrackHandle(track.handle().id_proto());
            FfiRequest request;
            request.setNewAudioStream(audioStreamRequest);
            QProtobufSerializer serializer;
            auto data = request.serialize(&serializer);
            const uint8_t *ret_data;
            size_t size;
            livekit_ffi_request((const uint8_t *)data.data(), data.length(), &ret_data, &size);
            FfiResponse newResponse;
            newResponse.deserialize(&serializer, QByteArray::fromRawData((const char *)ret_data, size));
        } else if (track.info().kind() == TrackKindGadget::KIND_VIDEO) {
            NewVideoStreamRequest videoStreamRequest;
            videoStreamRequest.setTrackHandle((track.handle().id_proto()));
            FfiRequest request;
            request.setNewVideoStream(videoStreamRequest);
            QProtobufSerializer serializer;
            auto data = request.serialize(&serializer);
            const uint8_t *ret_data;
            size_t size;
            livekit_ffi_request((const uint8_t *)data.data(), data.length(), &ret_data, &size);
            FfiResponse newResponse;
            newResponse.deserialize(&serializer, QByteArray::fromRawData((const char *)ret_data, size));
        }
    } else if (event.hasTrackUnsubscribed()) {
        qWarning() << "Track unsubscribed";
    } else if (event.hasTrackSubscriptionFailed()) {
        qWarning() << "Track subscription failed";
    } else if (event.hasTrackMuted()) {
        qWarning() << "Track muted";
    } else if (event.hasTrackUnmuted()) {
        qWarning() << "Track unmuted";
    } else if (event.hasActiveSpeakersChanged()) {
        qWarning() << "Active speakers changed";
    } else if (event.hasRoomMetadataChanged()) {
        qWarning() << "room metadata changed";
    } else if (event.hasParticipantMetadataChanged()) {
        qWarning() << "participant metadata changed";
    } else if (event.hasParticipantNameChanged()) {
        qWarning() << "participant name changed";
    } else if (event.hasConnectionQualityChanged()) {
        qWarning() << "connection quality changed to" << event.connectionQualityChanged().quality();
    } else if (event.hasDataPacketReceived()) {
        qWarning() << "data received";
    } else if (event.hasConnectionStateChanged()) {
        qWarning() << "connection state changed";
    } else if (event.hasDisconnected()) {
        qWarning() << "disconnected";
    } else if (event.hasReconnecting()) {
        qWarning() << "reconnecting";
    } else if (event.hasReconnected()) {
        qWarning() << "Reconnected";
    } else if (event.hasE2eeStateChanged()) {
        qWarning() << "e2eeStateChanged";
    } else if (event.hasEos()) {
        qWarning() << "eos";
    } else {
        qWarning() << "Unknown room event";
    }
}

void saveByteArray(const QByteArray &data, const QString &name)
{
    QFile file("/home/tobias/"_ls + name);
    file.open(QFile::WriteOnly);
    file.write(data);
    file.close();
}

void CallController::handleEvent(FfiEvent &&event)
{
    if (event.hasLogs()) {
        handleLog(std::move(event.logs().records()));
    } else if (event.hasRoomEvent()) {
        handleRoomEvent(std::move(event.roomEvent()));
    } else if (event.hasTrackEvent()) {
        qWarning() << "track event";
    } else if (event.hasVideoStreamEvent()) {
        qWarning() << "video stream event";
        auto video = event.videoStreamEvent();
        auto info = video.frameReceived().buffer().info();
        QByteArray data((const char *)info.dataPtr(), info.width() * info.height() * 1.5);
        auto frame = QVideoFrame(QVideoFrameFormat(QSize(info.width(), info.height()), QVideoFrameFormat::Format_YUV420P));
        frame.map(QVideoFrame::WriteOnly);
        memcpy(frame.bits(0), data.constData(), data.size() / 3 * 2);
        memcpy(frame.bits(1), data.constData() + data.size() / 3 * 2, data.size() / 6);
        memcpy(frame.bits(2), data.constData() + data.size() / 3 * 2 + data.size() / 6, data.size() / 6);
        qWarning() << frame.size() << data.toBase64();
        frame.unmap();
        m_sink->setVideoFrame(frame);
        delete (char *)info.dataPtr();
    } else if (event.hasAudioStreamEvent()) {
        return; //TODO remove
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            QAudioFormat format;
            format.setSampleRate(48000);
            format.setChannelCount(2);
            format.setSampleFormat(QAudioFormat::Int16);
            QAudioDevice info(QMediaDevices::defaultAudioOutput());
            if (!info.isFormatSupported(format)) {
                qWarning() << "Audio format not supported";
                Q_ASSERT(false);
                return;
            }
            sink = new QAudioSink(format);
            audioData = sink->start();
            QProtobufSerializer serializer;
            NewAudioResamplerRequest narr;
            FfiRequest request;
            request.setNewAudioResampler(narr);
            auto data = request.serialize(&serializer);
            const uint8_t *ret_data;
            size_t size;
            livekit_ffi_request((const uint8_t *)data.data(), data.length(), &ret_data, &size);
            FfiResponse newResponse;
            newResponse.deserialize(&serializer, QByteArray::fromRawData((const char *)ret_data, size));
            resampler = newResponse.newAudioResampler().resampler().handle().id_proto();
        }

        if (event.audioStreamEvent().hasFrameReceived()) {
            FfiRequest request;
            RemixAndResampleRequest rarr;
            rarr.setBuffer(event.audioStreamEvent().frameReceived().frame().info());
            rarr.setNumChannels(2);
            rarr.setSampleRate(48000);
            rarr.setResamplerHandle(resampler);
            request = FfiRequest();
            request.setRemixAndResample(rarr);
            static QProtobufSerializer serializer;
            auto data = request.serialize(&serializer);
            const uint8_t *ret_data;
            size_t size;
            livekit_ffi_request((const uint8_t *)data.data(), data.length(), &ret_data, &size);
            FfiResponse response;
            response.deserialize(&serializer, QByteArray::fromRawData((const char *)ret_data, size));
            Q_ASSERT(response.hasRemixAndResample());

            auto info = response.remixAndResample().buffer().info();
            auto bytes = info.numChannels() * info.samplesPerChannel() * 2;
            data = QByteArray::fromRawData((const char *)info.dataPtr(), bytes);
            audioData->write(data);
        }
    } else if (event.hasConnect()) {
        handleConnect(std::move(event.connect()));
    } else if (event.hasDisconnect()) {
        qWarning() << "disconnect";
    } else if (event.hasDispose()) {
        handleDispose(std::move(event.dispose()));
    } else if (event.hasPublishTrack()) {
        qWarning() << "publish track";
    } else if (event.hasUnpublishTrack()) {
        qWarning() << "unpublish track";
    } else if (event.hasPublishData()) {
        qWarning() << "publish data";
    } else if (event.hasCaptureAudioFrame()) {
        qWarning() << "audio frame";
    } else if (event.hasGetStats()) {
        qWarning() << "get stats";
    } else if (event.hasGetSessionStats()) {
        qWarning() << "get session stats";
    } else if (event.hasPanic()) {
        qWarning() << "panic";
    } else {
        qWarning() << event.messageField();
    }
}

void CallController::handleCallMemberEvent(const Quotient::CallMemberEvent *event, NeoChatRoom *room)
{
    qWarning() << event->fullJson();
    Q_EMIT callStarted();
    const auto connection = room->connection();
    auto job = connection->callApi<RequestOpenIdTokenJob>(connection->userId());
    connect(job, &BaseJob::finished, this, [this, room, job, connection, event]() {
        auto nam = new QNetworkAccessManager;
        auto json = QJsonDocument(QJsonObject{
                                      {"room"_ls, room->id()},
                                      {"openid_token"_ls,
                                       QJsonObject{{"access_token"_ls, job->tokenData().accessToken},
                                                   {"token_type"_ls, job->tokenData().tokenType},
                                                   {"matrix_server_name"_ls, job->tokenData().matrixServerName}}},
                                      {"device_id"_ls, connection->deviceId()},
                                  })
                        .toJson();
        // This is an old event!
        if (!event->contentJson().contains("foci_preferred"_ls)) {
            return;
        }
        QNetworkRequest request(QUrl((event->contentJson()["foci_preferred"_ls].toArray()[0]["livekit_service_url"_ls].toString() + "/sfu/get"_ls)));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"_ls);
        auto reply = nam->post(request, json);
        connect(reply, &QNetworkReply::finished, this, [reply, this, room]() {
            auto json = QJsonDocument::fromJson(reply->readAll()).object();

            FfiRequest message;
            ConnectRequest connectRequest;
            connectRequest.setUrl(json["url"_ls].toString());
            connectRequest.setToken(json["jwt"_ls].toString());
            message.setConnect(connectRequest);
            QProtobufSerializer serializer;
            auto data = message.serialize(&serializer);
            size_t size;
            const uint8_t *ret_data;
            livekit_ffi_request((const uint8_t *)data.data(), data.length(), &ret_data, &size);
            FfiResponse connectResponse;
            connectResponse.deserialize(&serializer, QByteArray::fromRawData((const char *)ret_data, size));
            if (!connectResponse.hasConnect()) {
                qWarning() << "connectResponse has unexpected content" << connectResponse.messageField();
                return;
            }
            m_connectingRooms[connectResponse.connect().asyncId()] = room;
        });
    });
}

FfiResponse request(FfiRequest &&request)
{
    static QProtobufSerializer serializer;
    auto data = request.serialize(&serializer);
    size_t responseLength;
    const char *responseData;
    livekit_ffi_request((const uint8_t *)data.constData(), data.size(), (const uint8_t **)&responseData, &responseLength);
    auto response = QByteArray::fromRawData(responseData, responseLength);
    FfiResponse ffiResponse;
    ffiResponse.deserialize(&serializer, response);
    return ffiResponse;
}

void CallController::setCameraVideoSink(QVideoSink *videoSink)
{
    m_cameraVideoSink = videoSink;
    connect(videoSink, &QVideoSink::videoFrameChanged, this, [videoSink, this]() {
        static bool initialized = false;
        if (localParticipant == 100000) {
            return; // TODO make less shitty
        }
        static QtProtobuf::uint64 handle;
        if (!initialized) {
            initialized = true;

            NewVideoSourceRequest newVideoSourceRequest;
            VideoSourceResolution resolution;
            resolution.setHeight(videoSink->videoSize().height());
            resolution.setWidth(videoSink->videoSize().width());
            newVideoSourceRequest.setResolution(resolution);
            newVideoSourceRequest.setType(VideoSourceTypeGadget::VIDEO_SOURCE_NATIVE);
            FfiRequest ffiRequest;
            ffiRequest.setNewVideoSource(newVideoSourceRequest);
            auto response = request(std::move(ffiRequest));
            handle = response.newVideoSource().source().handle().id_proto();
            m_localVideoTrackHandle = handle;

            CreateVideoTrackRequest createVideoTrackRequest;
            createVideoTrackRequest.setName("Camera"_ls);
            createVideoTrackRequest.setSourceHandle(handle);

            FfiRequest request;
            request.setCreateVideoTrack(createVideoTrackRequest);

            auto createResponse = ::request(std::move(request));
            m_localVideoTrackId = createResponse.createVideoTrack().track().handle().id_proto();
            publishTrack(m_localVideoTrackId);
        }

        auto image = videoSink->videoFrame().toImage();
        image.convertTo(QImage::Format_RGB888);
        CaptureVideoFrameRequest request;
        VideoBufferInfo buffer;
        buffer.setType(VideoBufferTypeGadget::RGB24);
        buffer.setWidth(image.width());
        buffer.setHeight(image.height());
        buffer.setDataPtr((QtProtobuf::uint64)image.bits());
        buffer.setStride(image.bytesPerLine());
        VideoBufferInfo_QtProtobufNested::ComponentInfoRepeated components;
        VideoBufferInfo_QtProtobufNested::ComponentInfo componentInfo;
        componentInfo.setStride(image.bytesPerLine());
        componentInfo.setDataPtr((QtProtobuf::uint64)image.bits());
        componentInfo.setSize(image.sizeInBytes());
        components += componentInfo;
        buffer.setComponents(components);
        request.setBuffer(buffer);
        request.setSourceHandle(handle);
        request.setTimestampUs(QDateTime::currentMSecsSinceEpoch() * 1000);
        request.setRotation(VideoRotationGadget::VIDEO_ROTATION_0);
        FfiRequest ffiRequest;
        ffiRequest.setCaptureVideoFrame(request);
        auto response = ::request(std::move(ffiRequest));
    });
}

void CallController::setVideoSink(QObject *sink)
{
    m_sink = dynamic_cast<QVideoSink *>(sink);
}

void LivekitVideoSink::setVideoSink(QVideoSink *videoSink)
{
    m_videoSink = videoSink;
    CallController::instance().setVideoSink(videoSink);
    Q_EMIT videoSinkChanged();
}
QVideoSink *LivekitVideoSink::videoSink() const
{
    return m_videoSink;
}

void CallController::toggleCamera()
{
    if (m_localVideoTrackSid.isEmpty()) {
        publishTrack(m_localVideoTrackId);
    } else {
        FfiRequest request;
        UnpublishTrackRequest unpublishRequest;
        unpublishRequest.setLocalParticipantHandle(localParticipant);
        unpublishRequest.setTrackSid(m_localVideoTrackSid);
        request.setUnpublishTrack(unpublishRequest);
        auto response = ::request(std::move(request));
        m_localVideoTrackSid = QString();
    }
}

void CallController::publishTrack(uint64_t id)
{

    PublishTrackRequest publishTrackRequest;
    publishTrackRequest.setTrackHandle(id);
    publishTrackRequest.setLocalParticipantHandle(localParticipant);
    TrackPublishOptions options;
    options.setSource(TrackSourceGadget::SOURCE_CAMERA);
    options.setVideoCodec(VideoCodecGadget::VideoCodec::VP8);
    publishTrackRequest.setOptions(options);

    auto request = FfiRequest();
    request.setPublishTrack(publishTrackRequest);
    auto publishResponse = ::request(std::move(request));
}
