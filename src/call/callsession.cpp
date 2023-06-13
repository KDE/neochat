// SPDX-FileCopyrightText: 2021 Nheko Contributors
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2021-2022 Tobias Fella <fella@posteo.de>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "calldevices.h"

#include <QDebug>
#include <QThread>

#include <gst/gst.h>

#define GST_USE_UNSTABLE_API
#include <gst/webrtc/webrtc.h>
#undef GST_USE_UNSTABLE_API

#include "voiplogging.h"

#include "audiosources.h"
#include "videosources.h"

#include <qcoro/qcorosignal.h>

#define private public
#include "callsession.h"
#undef private
#include "callmanager.h"
#include <qt_connection_util.h>

#define STUN_SERVER "stun://turn.matrix.org:3478" // TODO make STUN server configurable

#define INSTANCE                                                                                                                                               \
    Q_ASSERT(user_data);                                                                                                                                       \
    auto instance = static_cast<CallSession *>(user_data);

GstElement *createElement(const char *type, GstElement *pipe, const char *name = nullptr)
{
    auto element = gst_element_factory_make(type, name);
    Q_ASSERT_X(element, __FUNCTION__, QStringLiteral("Failed to create element %1 %2").arg(type, name).toLatin1());
    if (pipe) {
        gst_bin_add_many(GST_BIN(pipe), element, nullptr);
    }
    return element;
}

GstElement *binGetByName(GstElement *bin, const char *name)
{
    auto element = gst_bin_get_by_name(GST_BIN(bin), name);
    Q_ASSERT_X(element, __FUNCTION__, QStringLiteral("Failed to get element by name: %1").arg(name).toLatin1());
    return element;
}

struct KeyFrameRequestData {
    GstElement *pipe = nullptr;
    GstElement *decodeBin = nullptr;
    gint packetsLost = 0;
    guint timerId = 0;
    QString statsField;
} keyFrameRequestData;

std::pair<int, int> getResolution(GstPad *pad)
{
    std::pair<int, int> ret;
    auto caps = gst_pad_get_current_caps(pad);
    auto structure = gst_caps_get_structure(caps, 0);
    gst_structure_get_int(structure, "width", &ret.first);
    gst_structure_get_int(structure, "height", &ret.second);
    gst_caps_unref(caps);
    return ret;
}

std::pair<int, int> getResolution(GstElement *pipe, const gchar *elementName, const gchar *padName)
{
    auto element = binGetByName(pipe, elementName);
    auto pad = gst_element_get_static_pad(element, padName);
    auto ret = getResolution(pad);
    gst_object_unref(pad);
    gst_object_unref(element);
    return ret;
}

void setLocalDescription(GstPromise *promise, gpointer user_data)
{
    INSTANCE
    qCDebug(voip) << "Setting local description";
    const GstStructure *reply = gst_promise_get_reply(promise);
    gboolean isAnswer = gst_structure_id_has_field(reply, g_quark_from_string("answer"));
    GstWebRTCSessionDescription *gstsdp = nullptr;
    gst_structure_get(reply, isAnswer ? "answer" : "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &gstsdp, nullptr);
    gst_promise_unref(promise);
    auto webrtcbin = binGetByName(instance->m_pipe, "webrtcbin");
    Q_ASSERT(gstsdp);
    g_signal_emit_by_name(webrtcbin, "set-local-description", gstsdp, nullptr);
    gchar *sdp = gst_sdp_message_as_text(gstsdp->sdp);
    if (!instance->m_localSdp.isEmpty()) {
        // This is a renegotiation
        Q_EMIT instance->renegotiate(QString(sdp));
    }
    instance->m_localSdp = QString(sdp);
    g_free(sdp);
    gst_webrtc_session_description_free(gstsdp);
    qCDebug(voip) << "Local description set:" << isAnswer;
}

bool contains(std::string_view str1, std::string_view str2)
{
    return std::search(str1.cbegin(),
                       str1.cend(),
                       str2.cbegin(),
                       str2.cend(),
                       [](unsigned char c1, unsigned char c2) {
                           return std::tolower(c1) == std::tolower(c2);
                       })
        != str1.cend();
}

void createOffer(GstElement *webrtc, CallSession *session)
{
    // TODO ?!?
    if (!session->m_localSdp.isEmpty()) {
        return;
    }
    qCWarning(voip) << "Creating Offer";
    auto promise = gst_promise_new_with_change_func(setLocalDescription, session, nullptr);
    g_signal_emit_by_name(webrtc, "create-offer", nullptr, promise);
}

void createAnswer(GstPromise *promise, gpointer user_data)
{
    INSTANCE
    qCDebug(voip) << "Creating Answer";
    gst_promise_unref(promise);
    promise = gst_promise_new_with_change_func(setLocalDescription, instance, nullptr);
    auto webrtcbin = binGetByName(instance->m_pipe, "webrtcbin");
    g_signal_emit_by_name(webrtcbin, "create-answer", nullptr, promise);
}

bool getMediaAttributes(const GstSDPMessage *sdp, const char *mediaType, const char *encoding, int &payloadType, bool &receiveOnly, bool &sendOnly)
{
    payloadType = -1;
    receiveOnly = false;
    sendOnly = false;
    for (guint mlineIndex = 0; mlineIndex < gst_sdp_message_medias_len(sdp); mlineIndex++) {
        const GstSDPMedia *media = gst_sdp_message_get_media(sdp, mlineIndex);
        if (!strcmp(gst_sdp_media_get_media(media), mediaType)) {
            receiveOnly = gst_sdp_media_get_attribute_val(media, "recvonly") != nullptr;
            sendOnly = gst_sdp_media_get_attribute_val(media, "sendonly") != nullptr;
            const gchar *rtpval = nullptr;
            for (guint n = 0; n == 0 || rtpval; n++) {
                rtpval = gst_sdp_media_get_attribute_val_n(media, "rtpmap", n);
                if (rtpval && contains(rtpval, encoding)) {
                    payloadType = QString::fromLatin1(rtpval).toInt();
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

GstWebRTCSessionDescription *parseSDP(const QString &sdp, GstWebRTCSDPType type)
{
    GstSDPMessage *message;
    gst_sdp_message_new(&message);
    if (gst_sdp_message_parse_buffer((guint8 *)sdp.toLatin1().data(), sdp.size(), message) == GST_SDP_OK) {
        return gst_webrtc_session_description_new(type, message);
    } else {
        qCCritical(voip) << "Failed to parse remote SDP";
        gst_sdp_message_free(message);
        return nullptr;
    }
}

void addLocalICECandidate(GstElement *webrtc, guint mlineIndex, const gchar *candidate, gpointer user_data)
{
    Q_UNUSED(webrtc);
    INSTANCE
    // qCWarning(voip) << "Adding local ICE Candidates";
    instance->m_localCandidates += Candidate{candidate, static_cast<int>(mlineIndex), QString()};
}

void iceConnectionStateChanged(GstElement *webrtc, GParamSpec *pspec, gpointer user_data)
{
    Q_UNUSED(pspec);
    INSTANCE
    GstWebRTCICEConnectionState newState;
    g_object_get(webrtc, "ice-connection-state", &newState, nullptr);
    switch (newState) {
    case GST_WEBRTC_ICE_CONNECTION_STATE_NEW:
    case GST_WEBRTC_ICE_CONNECTION_STATE_CHECKING:
        instance->setState(CallSession::CONNECTING);
        break;
    case GST_WEBRTC_ICE_CONNECTION_STATE_FAILED:
        instance->setState(CallSession::ICEFAILED);
        break;
    case GST_WEBRTC_ICE_CONNECTION_STATE_COMPLETED:
    case GST_WEBRTC_ICE_CONNECTION_STATE_DISCONNECTED:
    case GST_WEBRTC_ICE_CONNECTION_STATE_CONNECTED:
    case GST_WEBRTC_ICE_CONNECTION_STATE_CLOSED:
    default:
        break;
    }
}

GstElement *newAudioSinkChain(GstElement *pipe)
{
    qCWarning(voip) << "New Audio Sink Chain";
    GstElement *queue = createElement("queue", pipe);
    GstElement *convert = createElement("audioconvert", pipe);
    GstElement *resample = createElement("audioresample", pipe);
    GstElement *sink = createElement("autoaudiosink", pipe);
    gst_element_link_many(queue, convert, resample, sink, nullptr);
    gst_element_sync_state_with_parent(queue);
    gst_element_sync_state_with_parent(convert);
    gst_element_sync_state_with_parent(resample);
    gst_element_sync_state_with_parent(sink);
    return queue;
}

void sendKeyFrameRequest()
{
    auto sinkpad = gst_element_get_static_pad(keyFrameRequestData.decodeBin, "sink");
    if (!gst_pad_push_event(sinkpad, gst_event_new_custom(GST_EVENT_CUSTOM_UPSTREAM, gst_structure_new_empty("GstForceKeyUnit")))) {
        qCWarning(voip) << "Keyframe request failed";
    }
    gst_object_unref(sinkpad);
}

void onGetStats(GstPromise *promise, gpointer)
{
    auto reply = gst_promise_get_reply(promise);
    GstStructure *rtpStats;
    if (!gst_structure_get(reply, keyFrameRequestData.statsField.toLatin1().data(), GST_TYPE_STRUCTURE, &rtpStats, nullptr)) {
        gst_promise_unref(promise);
        return;
    }
    auto packetsLost = 0;
    gst_structure_get_int(rtpStats, "packets-lost", &packetsLost);
    gst_structure_free(rtpStats);
    gst_promise_unref(promise);
    if (packetsLost > keyFrameRequestData.packetsLost) {
        qCWarning(voip) << "inbound video lost packet count:" << packetsLost;
        keyFrameRequestData.packetsLost = packetsLost;
        sendKeyFrameRequest();
    }
}

// TODO port to QTimer?
gboolean testPacketLoss(gpointer)
{
    if (!keyFrameRequestData.pipe) {
        return false;
    }

    auto webrtc = binGetByName(keyFrameRequestData.pipe, "webrtcbin");
    auto promise = gst_promise_new_with_change_func(onGetStats, nullptr, nullptr);
    g_signal_emit_by_name(webrtc, "get-stats", nullptr, promise);
    gst_object_unref(webrtc);
    return true;
}

GstElement *newVideoSinkChain(GstElement *pipe, QQuickItem *quickItem)
{
    qCWarning(voip) << "Creating Video Sink Chain";
    auto queue = createElement("queue", pipe);
    auto compositor = createElement("compositor", pipe);
    auto glupload = createElement("glupload", pipe);
    auto glcolorconvert = createElement("glcolorconvert", pipe);
    auto qmlglsink = createElement("qmlglsink", nullptr);
    auto glsinkbin = createElement("glsinkbin", pipe);
    g_object_set(qmlglsink, "widget", quickItem, nullptr);
    g_object_set(glsinkbin, "sink", qmlglsink, nullptr);
    gst_element_link_many(queue, compositor, glupload, glcolorconvert, glsinkbin, nullptr);
    gst_element_sync_state_with_parent(queue);
    gst_element_sync_state_with_parent(compositor);
    gst_element_sync_state_with_parent(glupload);
    gst_element_sync_state_with_parent(glcolorconvert);
    gst_element_sync_state_with_parent(glsinkbin);
    return queue;
}

static GstPadProbeReturn pad_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    Q_UNUSED(pad);
    // auto stream = static_cast<VideoStream *>(user_data);
    auto event = GST_PAD_PROBE_INFO_EVENT(info);
    if (GST_EVENT_CAPS == GST_EVENT_TYPE(event)) {
        GstCaps *caps = gst_caps_new_any();
        int width, height;
        gst_event_parse_caps(event, &caps);
        auto structure = gst_caps_get_structure(caps, 0);
        gst_structure_get_int(structure, "width", &width);
        gst_structure_get_int(structure, "height", &height);
        // stream->setWidth(width);
        // stream->setHeight(height);
        // TODO needed?
    }
    return GST_PAD_PROBE_OK;
}

void linkNewPad(GstElement *decodeBin, GstPad *newpad, gpointer user_data)
{
    INSTANCE
    qCWarning(voip) << "Linking New Pad";
    auto sinkpad = gst_element_get_static_pad(decodeBin, "sink");
    auto sinkcaps = gst_pad_get_current_caps(sinkpad);
    auto structure = gst_caps_get_structure(sinkcaps, 0);

    gchar *mediaType = nullptr;
    guint ssrc = 0;
    gst_structure_get(structure, "media", G_TYPE_STRING, &mediaType, "ssrc", G_TYPE_UINT, &ssrc, nullptr);
    gst_caps_unref(sinkcaps);
    gst_object_unref(sinkpad);

    GstElement *queue = nullptr;
    if (!strcmp(mediaType, "audio")) {
        qCWarning(voip) << "Receiving audio stream";
        queue = newAudioSinkChain(instance->m_pipe);
    } else if (!strcmp(mediaType, "video")) {
        qCWarning(voip) << "Receiving video stream";
        auto fake = createElement("fakesink", instance->m_pipe);
        auto selector = createElement("output-selector", instance->m_pipe);
        auto selectorSink = gst_element_get_static_pad(selector, "sink");
        auto selectorSrc1 = gst_element_request_pad_simple(selector, "src_%u");
        gst_pad_link(newpad, selectorSink);
        auto fakepad = gst_element_get_static_pad(fake, "sink");
        gst_pad_link(selectorSrc1, fakepad);
        g_object_set(selector, "active-pad", selectorSrc1, nullptr);

        auto msid = instance->ssrcToMsid[ssrc];

        // gst_pad_add_probe(newpad, GST_PAD_PROBE_TYPE_EVENT_BOTH, pad_cb, stream, nullptr);
        auto manager = dynamic_cast<CallManager *>(instance->parent());
        auto participants = manager->callParticipants();
        auto user = dynamic_cast<NeoChatUser *>(manager->room()->user(instance->msidToUserId[msid]));
        participants->setHasCamera(user, true);

        auto participant = participants->callParticipantForUser(user);

        // gst_pad_add_probe(newpad, GST_PAD_PROBE_TYPE_EVENT_BOTH, pad_cb, nullptr, nullptr);
        connectSingleShot(participant, &CallParticipant::initialized, instance, [=](QQuickItem *item) {
            gst_pad_unlink(newpad, fakepad);
            auto queue = newVideoSinkChain(instance->m_pipe, item);
            auto queuepad = gst_element_get_static_pad(queue, "sink");
            Q_ASSERT(queuepad);
            auto selectorSrc = gst_element_request_pad_simple(selector, "src_%u");
            auto ok = GST_PAD_LINK_SUCCESSFUL(gst_pad_link(selectorSrc, queuepad));
            Q_ASSERT(ok);
            g_object_set(selector, "active-pad", selectorSrc, nullptr);
            instance->setState(CallSession::CONNECTED);
            keyFrameRequestData.pipe = instance->m_pipe;
            keyFrameRequestData.decodeBin = decodeBin;
            keyFrameRequestData.timerId = g_timeout_add_seconds(3, testPacketLoss, nullptr);
            keyFrameRequestData.statsField = QStringLiteral("rtp-inbound-stream-stats_") + QString::number(ssrc);
            gst_object_unref(queuepad);
            g_free(mediaType);
        });
        return;
    } else {
        g_free(mediaType);
        qCWarning(voip) << "Unknown pad type:" << GST_PAD_NAME(newpad);
        return;
    }
    auto queuepad = gst_element_get_static_pad(queue, "sink");
    Q_ASSERT(queuepad);
    auto ok = GST_PAD_LINK_SUCCESSFUL(gst_pad_link(newpad, queuepad));
    Q_ASSERT(ok);
    gst_object_unref(queuepad);
    g_free(mediaType);
}

void setWaitForKeyFrame(GstBin *decodeBin, GstElement *element, gpointer)
{
    Q_UNUSED(decodeBin);
    if (!strcmp(gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(gst_element_get_factory(element))), "rtpvp8depay")) {
        g_object_set(element, "wait-for-keyframe", TRUE, nullptr);
    }
}

void addDecodeBin(GstElement *webrtc, GstPad *newpad, gpointer user_data)
{
    Q_UNUSED(webrtc);
    if (GST_PAD_DIRECTION(newpad) != GST_PAD_SRC) {
        return;
    }

    INSTANCE

    auto decodeBin = createElement("decodebin", instance->m_pipe);
    // Investigate hardware, see nheko source
    g_object_set(decodeBin, "force-sw-decoders", TRUE, nullptr);
    g_signal_connect(decodeBin, "pad-added", G_CALLBACK(linkNewPad), instance);
    g_signal_connect(decodeBin, "element-added", G_CALLBACK(setWaitForKeyFrame), nullptr);
    gst_element_sync_state_with_parent(decodeBin);
    auto sinkpad = gst_element_get_static_pad(decodeBin, "sink");
    if (GST_PAD_LINK_FAILED(gst_pad_link(newpad, sinkpad))) {
        // TODO: Error handling
        qCWarning(voip) << "Unable to link decodebin";
    }
    gst_object_unref(sinkpad);
}

void iceGatheringStateChanged(GstElement *webrtc, GParamSpec *pspec, gpointer user_data)
{
    Q_UNUSED(pspec);
    INSTANCE

    GstWebRTCICEGatheringState newState;
    g_object_get(webrtc, "ice-gathering-state", &newState, nullptr);
    if (newState == GST_WEBRTC_ICE_GATHERING_STATE_COMPLETE) {
        qCWarning(voip) << "GstWebRTCICEGatheringState -> Complete";
        if (instance->m_isOffering) {
            Q_EMIT instance->offerCreated(instance->m_localSdp, instance->m_localCandidates);
            instance->setState(CallSession::OFFERSENT);
        } else {
            Q_EMIT instance->answerCreated(instance->m_localSdp, instance->m_localCandidates);
            instance->setState(CallSession::ANSWERSENT);
        }
    }
}

gboolean newBusMessage(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    Q_UNUSED(bus);
    INSTANCE

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        qCWarning(voip) << "End of stream";
        // TODO: Error handling
        instance->end();
        break;
    case GST_MESSAGE_ERROR:
        GError *error;
        gchar *debug;
        gst_message_parse_error(msg, &error, &debug);
        qCWarning(voip) << "Error from element:" << GST_OBJECT_NAME(msg->src) << error->message;
        // TODO: Error handling
        g_clear_error(&error);
        g_free(debug);
        instance->end();
        break;
    default:
        break;
    }
    return TRUE;
}

CallSession::CallSession(QObject *parent)
    : QObject(parent)
{
}

void CallSession::acceptAnswer(const QString &sdp, const QVector<Candidate> &candidates, const QString &userId)
{
    qCDebug(voip) << "Accepting Answer";
    if (m_state != CallSession::OFFERSENT) {
        return;
    }

    GstWebRTCSessionDescription *answer = parseSDP(sdp, GST_WEBRTC_SDP_TYPE_ANSWER);
    if (!answer) {
        end();
        return;
    }

    acceptCandidates(candidates);

    setRemoteDescription(answer, userId);
}

void CallSession::setRemoteDescription(GstWebRTCSessionDescription *remote, const QString &userId, GstPromise *promise)
{
    GstElement *webrtcbin = binGetByName(m_pipe, "webrtcbin");
    auto sdp = remote->sdp;
    for (guint i = 0; i < gst_sdp_message_medias_len(sdp); i++) {
        auto media = gst_sdp_message_get_media(sdp, i);
        QList<uint32_t> ssrcs;
        QString msid;
        for (guint j = 0; j < gst_sdp_media_attributes_len(media); j++) {
            auto attribute = gst_sdp_media_get_attribute(media, j);
            if (!strcmp(attribute->key, "ssrc")) {
                ssrcs += QString(attribute->value).split(" ")[0].toUInt();
            }
            if (!strcmp(attribute->key, "msid")) {
                msid = QString(attribute->value).split(" ")[0];
            }
        }
        for (const auto &ssrc : ssrcs) {
            ssrcToMsid[ssrc] = msid;
        }
        msidToUserId[msid] = userId;
    }
    g_signal_emit_by_name(webrtcbin, "set-remote-description", remote, promise);
}

void CallSession::renegotiateOffer(const QString &_offer, const QString &userId)
{
    GstWebRTCSessionDescription *offer = parseSDP(_offer, GST_WEBRTC_SDP_TYPE_OFFER);
    if (!offer) {
        Q_ASSERT(false);
    }
    GstElement *webrtcbin = binGetByName(m_pipe, "webrtcbin");

    setRemoteDescription(offer, userId);
    GstPromise *promise = gst_promise_new_with_change_func(setLocalDescription, this, nullptr);
    g_signal_emit_by_name(webrtcbin, "create-answer", nullptr, promise);
}

void CallSession::acceptOffer(const QString &sdp, const QVector<Candidate> remoteCandidates, const QString &userId)
{
    Q_ASSERT(!sdp.isEmpty());
    Q_ASSERT(!remoteCandidates.isEmpty());
    qCDebug(voip) << "Accepting offer";
    if (m_state != CallSession::DISCONNECTED) {
        return;
    }
    m_isOffering = false;

    GstWebRTCSessionDescription *offer = parseSDP(sdp, GST_WEBRTC_SDP_TYPE_OFFER);
    if (!offer) {
        qCCritical(voip) << "Not an offer";
        return;
    }

    int opusPayloadType;
    bool receiveOnly;
    bool sendOnly;
    if (getMediaAttributes(offer->sdp, "audio", "opus", opusPayloadType, receiveOnly, sendOnly)) {
        if (opusPayloadType == -1) {
            qCCritical(voip) << "No OPUS in offer";
            gst_webrtc_session_description_free(offer);
            return;
        }
    } else {
        qCCritical(voip) << "No audio in offer";
        gst_webrtc_session_description_free(offer);
        return;
    }
    startPipeline();

    QThread::msleep(1000); // ?

    acceptCandidates(remoteCandidates);

    auto promise = gst_promise_new_with_change_func(createAnswer, this, nullptr);
    setRemoteDescription(offer, userId, promise);
    gst_webrtc_session_description_free(offer);
}

void CallSession::createCall()
{
    qCDebug(voip) << "Creating call";
    m_isOffering = true;
    startPipeline();
}

void CallSession::startPipeline()
{
    qCDebug(voip) << "Starting Pipeline";
    if (m_state != CallSession::DISCONNECTED) {
        return;
    }
    m_state = CallSession::INITIATING;
    Q_EMIT stateChanged();

    createPipeline();

    auto webrtcbin = binGetByName(m_pipe, "webrtcbin");
    Q_ASSERT(webrtcbin);
    if (false /*TODO: CHECK USE STUN*/) {
        qCDebug(voip) << "Setting STUN server:" << STUN_SERVER;
        g_object_set(webrtcbin, "stun-server", STUN_SERVER, nullptr);
    }

    for (const auto &uri : m_turnServers) {
        qCDebug(voip) << "Setting turn server:" << uri;
        gboolean udata;
        g_signal_emit_by_name(webrtcbin, "add-turn-server", uri.toLatin1().data(), (gpointer)(&udata));
    }

    if (m_turnServers.empty()) {
        qCWarning(voip) << "No TURN servers provided";
    }

    if (m_isOffering) {
        g_signal_connect(webrtcbin, "on-negotiation-needed", G_CALLBACK(::createOffer), this);
    }

    g_signal_connect(webrtcbin, "on-ice-candidate", G_CALLBACK(addLocalICECandidate), this);
    g_signal_connect(webrtcbin, "notify::ice-connection-state", G_CALLBACK(iceConnectionStateChanged), this);

    gst_element_set_state(m_pipe, GST_STATE_READY);
    g_signal_connect(webrtcbin, "pad-added", G_CALLBACK(addDecodeBin), this);

    g_signal_connect(webrtcbin, "notify::ice-gathering-state", G_CALLBACK(iceGatheringStateChanged), this);
    gst_object_unref(webrtcbin);

    GstStateChangeReturn ret = gst_element_set_state(m_pipe, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        // TODO: Error handling
        qCCritical(voip) << "Unable to start pipeline";
        end();
        return;
    }

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipe));
    m_busWatchId = gst_bus_add_watch(bus, newBusMessage, this);
    gst_object_unref(bus);

    m_state = CallSession::INITIATED;
    Q_EMIT stateChanged();
}

void CallSession::end()
{
    qCDebug(voip) << "Ending Call";
    if (m_pipe) {
        gst_element_set_state(m_pipe, GST_STATE_NULL);
        gst_object_unref(m_pipe);
        m_pipe = nullptr;
        keyFrameRequestData.pipe = nullptr;
        if (m_busWatchId) {
            g_source_remove(m_busWatchId);
            m_busWatchId = 0;
        }
    }
    if (m_state != CallSession::DISCONNECTED) {
        m_state = CallSession::DISCONNECTED;
        Q_EMIT stateChanged();
    }
}

void CallSession::createPipeline()
{
    qCWarning(voip) << "Creating Pipeline";
    auto device = AudioSources::instance().currentDevice();
    if (!device) {
        return;
    }
    m_pipe = gst_pipeline_new(nullptr);
    auto source = gst_device_create_element(device, nullptr);
    auto volume = createElement("volume", m_pipe, "srclevel");
    auto convert = createElement("audioconvert", m_pipe);
    auto resample = createElement("audioresample", m_pipe);
    auto queue1 = createElement("queue", m_pipe);
    auto opusenc = createElement("opusenc", m_pipe);
    auto rtp = createElement("rtpopuspay", m_pipe);
    auto queue2 = createElement("queue", m_pipe);
    auto capsfilter = createElement("capsfilter", m_pipe);

    auto rtpcaps = gst_caps_new_simple("application/x-rtp",
                                       "media",
                                       G_TYPE_STRING,
                                       "audio",
                                       "encoding-name",
                                       G_TYPE_STRING,
                                       "OPUS",
                                       "payload",
                                       G_TYPE_INT,
                                       OPUS_PAYLOAD_TYPE,
                                       nullptr);
    Q_ASSERT(rtpcaps);
    g_object_set(capsfilter, "caps", rtpcaps, nullptr);
    gst_caps_unref(rtpcaps);

    auto webrtcbin = createElement("webrtcbin", m_pipe, "webrtcbin");
    g_object_set(webrtcbin, "bundle-policy", GST_WEBRTC_BUNDLE_POLICY_MAX_BUNDLE, nullptr);

    gst_bin_add_many(GST_BIN(m_pipe), source, nullptr);

    if (!gst_element_link_many(source, volume, convert, resample, queue1, opusenc, rtp, queue2, capsfilter, webrtcbin, nullptr)) {
        qCCritical(voip) << "Failed to link pipeline";
        // TODO propagate errors up and end call
        return;
    }

    // if (sendVideo) {
    // TODO where?    addVideoPipeline();
    // }
}

void CallSession::toggleCamera()
{
    g_object_set(m_inputSelector, "active-pad", m_inactivePad, nullptr);
    auto _tmp = m_inactivePad;
    m_inactivePad = m_activePad;
    m_activePad = _tmp;
}

bool CallSession::addVideoPipeline()
{
    qCDebug(voip) << "Adding Video Pipeline";
    auto videoconvert = createElement("videoconvertscale", m_pipe);
    auto tee = createElement("tee", m_pipe);
    auto device = VideoSources::instance().currentDevice();
    auto deviceCaps = device->caps[VideoSources::instance().capsIndex()];
    int width = deviceCaps.width;
    int height = deviceCaps.height;
    int framerate = deviceCaps.framerates.back();
    if (!device) {
        return false;
    }
    auto camera = gst_device_create_element(device->device, nullptr);
    gst_bin_add_many(GST_BIN(m_pipe), camera, nullptr);

    auto caps =
        gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, "framerate", GST_TYPE_FRACTION, framerate, 1, nullptr);
    auto camerafilter = createElement("capsfilter", m_pipe);
    g_object_set(camerafilter, "caps", caps, nullptr);
    gst_caps_unref(caps);

    auto videotestsrc = createElement("videotestsrc", m_pipe);
    m_inputSelector = createElement("input-selector", m_pipe);
    g_object_set(m_inputSelector, "sync-mode", 1, nullptr);

    m_inactivePad = gst_element_request_pad_simple(m_inputSelector, "sink_%u");
    gst_pad_link(gst_element_get_static_pad(videotestsrc, "src"), m_inactivePad);

    auto selectorSrc = gst_element_get_static_pad(m_inputSelector, "src");
    gst_pad_link(selectorSrc, gst_element_get_static_pad(videoconvert, "sink"));

    m_activePad = gst_element_request_pad_simple(m_inputSelector, "sink_%u");
    gst_pad_link(gst_element_get_static_pad(camera, "src"), m_activePad);
    g_object_set(m_inputSelector, "active-pad", m_activePad, nullptr);

    if (!gst_element_link_many(videoconvert, camerafilter, nullptr)) {
        qCWarning(voip) << "Failed to link camera elements";
        // TODO: Error handling
        return false;
    }
    if (!gst_element_link(camerafilter, tee)) {
        qCWarning(voip) << "Failed to link camerafilter -> tee";
        // TODO: Error handling
        return false;
    }

    auto queue = createElement("queue", m_pipe);
    g_object_set(queue, "leaky", true, nullptr);
    auto vp8enc = createElement("vp8enc", m_pipe);
    g_object_set(vp8enc, "deadline", 1, nullptr);
    g_object_set(vp8enc, "error-resilient", 1, nullptr);
    auto rtpvp8pay = createElement("rtpvp8pay", m_pipe);
    auto rtpqueue = createElement("queue", m_pipe);
    auto rtpcapsfilter = createElement("capsfilter", m_pipe);
    auto rtpcaps = gst_caps_new_simple("application/x-rtp",
                                       "media",
                                       G_TYPE_STRING,
                                       "video",
                                       "encoding-name",
                                       G_TYPE_STRING,
                                       "VP8",
                                       "payload",
                                       G_TYPE_INT,
                                       VP8_PAYLOAD_TYPE,
                                       nullptr);
    g_object_set(rtpcapsfilter, "caps", rtpcaps, nullptr);
    gst_caps_unref(rtpcaps);

    auto webrtcbin = binGetByName(m_pipe, "webrtcbin");
    if (!gst_element_link_many(tee, queue, vp8enc, rtpvp8pay, rtpqueue, rtpcapsfilter, webrtcbin, nullptr)) {
        qCCritical(voip) << "Failed to link rtp video elements";
        gst_object_unref(webrtcbin);
        return false;
    }

    gst_object_unref(webrtcbin);

    auto newpad = gst_element_request_pad_simple(tee, "src_%u");
    Q_ASSERT(newpad);

    auto fake = createElement("fakesink", m_pipe);
    auto selector = createElement("output-selector", m_pipe);
    auto selectorSink = gst_element_get_static_pad(selector, "sink");
    auto selectorSrc1 = gst_element_request_pad_simple(selector, "src_%u");
    gst_pad_link(newpad, selectorSink);
    auto fakepad = gst_element_get_static_pad(fake, "sink");
    gst_pad_link(selectorSrc1, fakepad);
    g_object_set(selector, "active-pad", selectorSrc1, nullptr);

    // gst_pad_add_probe(newpad, GST_PAD_PROBE_TYPE_EVENT_BOTH, pad_cb, stream, nullptr);
    auto manager = dynamic_cast<CallManager *>(parent());
    auto participants = manager->callParticipants();
    auto user = dynamic_cast<NeoChatUser *>(manager->room()->localUser());
    participants->setHasCamera(user, true);

    auto participant = participants->callParticipantForUser(user);

    connectSingleShot(participant, &CallParticipant::initialized, this, [=](QQuickItem *item) {
        gst_pad_unlink(newpad, fakepad);

        auto queue = newVideoSinkChain(m_pipe, item);
        Q_ASSERT(queue);
        auto queuepad = gst_element_get_static_pad(queue, "sink");
        Q_ASSERT(queuepad);
        auto selectorSrc = gst_element_request_pad_simple(selector, "src_%u");
        Q_ASSERT(selectorSrc);
        auto ok = GST_PAD_LINK_SUCCESSFUL(gst_pad_link(selectorSrc, queuepad));
        Q_ASSERT(ok);
        g_object_set(selector, "active-pad", selectorSrc, nullptr);
        gst_object_unref(queuepad);
    });
    return true;
}

void CallSession::setTurnServers(QStringList servers)
{
    qCDebug(voip) << "Setting Turn Servers";
    qWarning() << "TURN SERVERS" << servers;
    m_turnServers = servers;
}

void CallSession::acceptCandidates(const QVector<Candidate> &candidates)
{
    qCDebug(voip) << "Accepting ICE Candidates";
    auto webrtcbin = binGetByName(m_pipe, "webrtcbin");
    for (const auto &c : candidates) {
        qCDebug(voip) << "Remote candidate:" << c.candidate << c.sdpMLineIndex;
        g_signal_emit_by_name(webrtcbin, "add-ice-candidate", c.sdpMLineIndex, c.candidate.toLatin1().data());
    }
}

QStringList CallSession::missingPlugins()
{
    GstRegistry *registry = gst_registry_get();
    static const QVector<QString> videoPlugins = {
        QLatin1String("compositor"),
        QLatin1String("opengl"),
        QLatin1String("qmlgl"),
        QLatin1String("rtp"),
        QLatin1String("videoconvertscale"),
        QLatin1String("vpx"),
    };
    static const QVector<QString> audioPlugins = {
        QStringLiteral("audioconvert"),
        QStringLiteral("audioresample"),
        QStringLiteral("autodetect"),
        QStringLiteral("dtls"),
        QStringLiteral("nice"),
        QStringLiteral("opus"),
        QStringLiteral("playback"),
        QStringLiteral("rtpmanager"),
        QStringLiteral("srtp"),
        QStringLiteral("volume"),
        QStringLiteral("webrtc"),
    };
    QStringList missingPlugins;
    for (const auto &pluginName : videoPlugins + audioPlugins) {
        auto plugin = gst_registry_find_plugin(registry, pluginName.toLatin1().data());
        if (!plugin) {
            missingPlugins << pluginName;
        }
        gst_object_unref(plugin);
    }
    return missingPlugins;
}

void CallSession::setMuted(bool muted)
{
    const auto srclevel = binGetByName(m_pipe, "srclevel");
    g_object_set(srclevel, "mute", muted, nullptr);
    gst_object_unref(srclevel);
    Q_EMIT mutedChanged();
}

bool CallSession::muted() const
{
    if (m_state < CallSession::CONNECTING) {
        return false;
    }
    if (!m_pipe) {
        return false;
    }
    const auto srclevel = binGetByName(m_pipe, "srclevel");
    bool muted;
    if (!srclevel) {
        return false;
    }
    g_object_get(srclevel, "mute", &muted, nullptr);
    // gst_object_unref(srclevel); //TODO why does this crash?
    return muted;
}

CallSession *
CallSession::acceptCall(const QString &sdp, const QVector<Candidate> &candidates, const QStringList &turnUris, const QString &userId, QObject *parent)
{
    auto instance = new CallSession(parent);
    instance->setTurnServers(turnUris);
    instance->acceptOffer(sdp, candidates, userId);
    return instance;
}

CallSession *CallSession::startCall(const QStringList &turnUris, QObject *parent)
{
    auto instance = new CallSession(parent);

    instance->setTurnServers(turnUris);
    instance->createCall();
    return instance;
}

CallSession::State CallSession::state() const
{
    return m_state;
}

void CallSession::setState(CallSession::State state)
{
    qCWarning(voip) << "Setting state" << state;
    m_state = state;
    Q_EMIT stateChanged();
}

void CallSession::setMetadata(QJsonObject metadata)
{
    m_metadata = metadata;
}
