// SPDX-FileCopyrightText: 2021 Nheko Contributors
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2021-2022 Tobias Fella <fella@posteo.de>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonObject>
#include <QMetaType>
#include <QObject>
#include <QQuickItem>
#include <QString>
#include <variant>
#define GST_USE_UNSTABLE_API
#include <gst/webrtc/webrtc.h>

#include <gst/gst.h>

#define OPUS_PAYLOAD_TYPE 111
#define VP8_PAYLOAD_TYPE 96

class CallDevices;
class VideoStream;

struct Candidate {
    QString candidate;
    int sdpMLineIndex;
    QString sdpMid;
};

Q_DECLARE_METATYPE(Candidate)
Q_DECLARE_METATYPE(QVector<Candidate>)

class CallSession : public QObject
{
    Q_OBJECT

public:
    enum State {
        DISCONNECTED,
        ICEFAILED,
        INITIATING,
        INITIATED,
        OFFERSENT,
        ANSWERSENT,
        CONNECTING,
        CONNECTED,
    };
    Q_ENUM(State);

    Q_PROPERTY(CallSession::State state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)

    // For outgoing calls
    static CallSession *startCall(const QStringList &turnUris, QObject *parent = nullptr);
    void acceptAnswer(const QString &sdp, const QVector<Candidate> &candidates, const QString &parent);

    // For incoming calls
    static CallSession *
    acceptCall(const QString &sdp, const QVector<Candidate> &candidates, const QStringList &turnUris, const QString &userId, QObject *parent = nullptr);

    void end();

    void renegotiateOffer(const QString &offer, const QString &userId);
    void setTurnServers(QStringList servers);

    QStringList missingPlugins() const;

    CallSession::State state() const;

    void toggleCamera();
    bool muted() const;
    void setMuted(bool muted);
    void setMetadata(QJsonObject metadata);
    void acceptCandidates(const QVector<Candidate> &candidates);

    QMap<QString, QString> msidToUserId;
Q_SIGNALS:
    void stateChanged();
    void offerCreated(const QString &sdp, const QVector<Candidate> &candidates);

    void answerCreated(const QString &sdp, const QVector<Candidate> &candidates);

    void mutedChanged();
    void newVideoStream(VideoStream *stream);

    void renegotiate(QString sdp);

private:
    CallSession(QObject *parent = nullptr);
    void acceptOffer(const QString &sdp, const QVector<Candidate> remoteCandidates, const QString &userId);
    void createCall();

    void setRemoteDescription(GstWebRTCSessionDescription *remote, const QString &userId, GstPromise *promise = nullptr);
    void startPipeline();
    void createPipeline();
    bool addVideoPipeline();

    void setState(CallSession::State state);
    GstPad *m_activePad;
    GstElement *m_inputSelector;
    CallSession::State m_state = CallSession::DISCONNECTED;
    unsigned int m_busWatchId = 0;
    QStringList m_turnServers;
    QVector<Candidate> m_localCandidates;
    QString m_localSdp;
    GstElement *m_pipe = nullptr;
    bool m_isOffering = false;
    QMap<int, QString> ssrcToMsid;
    QJsonObject m_metadata;
    GstPad *m_inactivePad;
};
