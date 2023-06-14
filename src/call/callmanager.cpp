// SPDX-FileCopyrightText: 2020-2021 Nheko Authors
// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "callmanager.h"

#include "controller.h"

#include <gst/gst.h>

#include "voiplogging.h"
#include <KLocalizedString>
#include <QDateTime>

#include <QMediaPlaylist>
#include <QMimeDatabase>
#include <qcoro/qcorosignal.h>
#include <qt_connection_util.h>

#include "neochatconfig.h"

#define CALL_VERSION "1"

CallManager::CallManager()
{
    init();
    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, [this] {
        updateTurnServers();
    });
}

QCoro::Task<void> CallManager::updateTurnServers()
{
    if (m_cachedTurnUrisValidUntil > QDateTime::currentDateTime()) {
        co_return;
    }
    Controller::instance().activeConnection()->getTurnServers();

    auto servers = co_await qCoro(Controller::instance().activeConnection(), &Connection::turnServersChanged);
    m_cachedTurnUrisValidUntil = QDateTime::currentDateTime().addSecs(servers["ttl"].toInt());

    const auto password = servers["password"].toString();
    const auto username = servers["username"].toString();
    const auto uris = servers["uris"].toArray();

    m_cachedTurnUris.clear();
    for (const auto &u : uris) {
        QString uri = u.toString();
        auto c = uri.indexOf(':');
        if (c == -1) {
            qCWarning(voip) << "Invalid TURN URI:" << uri;
            continue;
        }
        QString scheme = uri.left(c);
        if (scheme != "turn" && scheme != "turns") {
            qCWarning(voip) << "Invalid TURN scheme:" << scheme;
            continue;
        }
        m_cachedTurnUris += QStringLiteral("%1://%2:%3@%4").arg(scheme, QUrl::toPercentEncoding(username), QUrl::toPercentEncoding(password), uri.mid(c + 1));
    }
}

QString CallManager::callId() const
{
    return m_callId;
}

void CallManager::handleCallEvent(NeoChatRoom *room, const Quotient::RoomEvent *event)
{
    if (const auto &inviteEvent = eventCast<const CallInviteEvent>(event)) {
        handleInvite(room, inviteEvent);
    } else if (const auto &hangupEvent = eventCast<const CallHangupEvent>(event)) {
        handleHangup(room, hangupEvent);
    } else if (const auto &candidatesEvent = eventCast<const CallCandidatesEvent>(event)) {
        handleCandidates(room, candidatesEvent);
    } else if (const auto &answerEvent = eventCast<const CallAnswerEvent>(event)) {
        handleAnswer(room, answerEvent);
    } else if (const auto &negotiateEvent = eventCast<const CallNegotiateEvent>(event)) {
        handleNegotiate(room, negotiateEvent);
    }
}

void CallManager::checkStartCall()
{
    if ((m_incomingCandidates.isEmpty() && !m_incomingSdp.contains("candidates"_ls)) || m_incomingSdp.isEmpty()) {
        qCDebug(voip) << "Not ready to start this call yet";
        return;
    }
    m_session->acceptAnswer(m_incomingSdp, m_incomingCandidates, m_remoteUser->id());
    m_incomingCandidates.clear();
    m_incomingSdp.clear();
    setGlobalState(ACTIVE);
}

void CallManager::handleAnswer(NeoChatRoom *room, const Quotient::CallAnswerEvent *event)
{
    if (globalState() != OUTGOING) {
        qCDebug(voip) << "Ignoring answer while in state" << globalState();
        return;
    }

    if (event->callId() != m_callId) {
        qCDebug(voip) << "Ignoring answer for unknown call id" << event->callId() << ". Our call id is" << m_callId;
        return;
    }

    if (event->senderId() == room->localUser()->id() && partyId() == event->contentJson()["party_id"].toString()) {
        qCDebug(voip) << "Ignoring echo for answer";
        return;
    }

    if (event->senderId() == room->localUser()->id()) {
        qCDebug(voip) << "Call was accepted on a different device";
        // Show the user that call was accepted on a different device
        // Stop ringing
        return;
    }

    // TODO handle that MSC wrt to accepting on other devices
    m_session->setMetadata(event->contentJson()["org.matrix.msc3077.sdp_stream_metadata"].toObject());
    m_remotePartyId = event->contentJson()["party_id"].toString();
    m_incomingSdp = event->sdp();
    checkStartCall();
}

void CallManager::handleCandidates(NeoChatRoom *room, const Quotient::CallCandidatesEvent *event)
{
    // TODO what if candidates come before invite? this looks wrong
    if (globalState() == IDLE) {
        qCDebug(voip) << "Ignoring candidates in state" << globalState();
        return;
    }

    if (event->senderId() == room->localUser()->id()) {
        qCDebug(voip) << "Ignoring candidates sent by ourself";
        return;
    }

    if (globalState() == ACTIVE) {
        QVector<Candidate> candidates;
        for (const auto &candidate : event->candidates()) {
            candidates += Candidate{candidate.toObject()["candidate"].toString(),
                                    candidate.toObject()["sdpMLineIndex"].toInt(),
                                    candidate.toObject()["sdpMid"].toString()};
        }
        m_session->acceptCandidates(candidates);
        return;
    }

    qCDebug(voip) << "Storing" << event->candidates().size() << "incoming candidates";
    for (const auto &candidate : event->candidates()) {
        m_incomingCandidates +=
            Candidate{candidate.toObject()["candidate"].toString(), candidate.toObject()["sdpMLineIndex"].toInt(), candidate.toObject()["sdpMid"].toString()};
    }

    if (globalState() == OUTGOING) {
        checkStartCall();
    }
}

void CallManager::handleInvite(NeoChatRoom *room, const Quotient::CallInviteEvent *event)
{
    if (event->senderId() == room->localUser()->id()) {
        qCDebug(voip) << "Igoring invite sent by ourself";
        return;
    }
    if (globalState() != IDLE) {
        // TODO handle glare
        qCDebug(voip) << "Ignoring invite while already in a call";
        return;
    }

    if (event->originTimestamp() < QDateTime::currentDateTime().addSecs(-60)) {
        qCDebug(voip) << "Ignoring outdated invite; sent at:" << event->originTimestamp() << "current:" << QDateTime::currentDateTime();
        return;
    }

    setGlobalState(INCOMING);

    m_incomingSdp = event->sdp();
    setRemoteUser(dynamic_cast<NeoChatUser *>(room->user(event->senderId())));
    setRoom(room);
    setCallId(event->callId());
    setPartyId(generatePartyId());
    m_remotePartyId = event->contentJson()["party_id"].toString();
    setLifetime(event->lifetime());
    Q_EMIT incomingCall(remoteUser(), room, event->lifetime(), callId());
    ring(event->lifetime());
}

void CallManager::handleNegotiate(NeoChatRoom *room, const Quotient::CallNegotiateEvent *event)
{
    Q_UNUSED(room);
    if (event->callId() != m_callId) {
        qCDebug(voip) << "Ignoring negotiate for unknown call id" << event->callId() << ". Our call id is" << m_callId;
        return;
    }
    if (event->partyId() != m_remotePartyId) {
        qCDebug(voip) << "Ignoring negotiate for unknown party id" << event->partyId() << ". Remote party id is" << m_remotePartyId;
        return;
    }
    if (event->senderId() != m_remoteUser->id()) {
        qCDebug(voip) << "Ignoring negotiate for unknown user id" << event->senderId() << ". Remote user id is" << m_remoteUser->id();
        return;
    }
    // TODO DUPLICATES FFS
    m_session->setMetadata(event->contentJson()["org.matrix.msc3077.sdp_stream_metadata"].toObject());
    m_session->renegotiateOffer(event->sdp(), m_remoteUser->id(), event->contentJson()["description"]["type"] == QStringLiteral("answer"));
}

void CallManager::ring(int lifetime)
{
    // TODO put a better default ringtone in the kcfg
    // TODO which one? ship one? plasma-mobile-sounds?
    if (!QFileInfo::exists(NeoChatConfig::ringtone())) {
        qCWarning(voip) << "Ringtone file doesn't exist. Not audibly ringing";
        return;
    }
    auto ringtone = QUrl::fromLocalFile(NeoChatConfig::ringtone());
    m_playlist.setPlaybackMode(QMediaPlaylist::Loop);
    m_playlist.clear();
    m_ringPlayer.setPlaylist(&m_playlist);
    m_playlist.addMedia(ringtone);
    m_ringPlayer.play();
    QTimer::singleShot(lifetime, this, [this]() {
        stopRinging();
        Q_EMIT callEnded();
    });
}

void CallManager::stopRinging()
{
    m_ringPlayer.stop();
}

void CallManager::handleHangup(NeoChatRoom *room, const Quotient::CallHangupEvent *event)
{
    if (globalState() == IDLE) {
        qCDebug(voip) << "Ignoring hangup since we're not in a call";
        return;
    }

    if (event->senderId() == room->localUser()->id()) {
        qCDebug(voip) << "Ignoring hangup we sent ourselves";
        // TODO hangup-to-decline by different device?
        return;
    }

    if (event->callId() != m_callId) {
        qCDebug(voip) << "Hangup not for this call. Event's call id:" << event->callId() << ". Our call id" << m_callId;
        return;
    }

    stopRinging();
    if (m_session) {
        m_session->end();
        delete m_session;
    }
    setGlobalState(IDLE);
    Q_EMIT callEnded();
}

void CallManager::acceptCall()
{
    // TODO metadata for this case
    if (globalState() != INCOMING) {
        qCWarning(voip) << "Not accepting call while state is" << globalState();
        return;
    }

    stopRinging();

    if (!checkPlugins()) {
        qCCritical(voip) << "Missing plugins; can't accept call";
    }

    updateTurnServers();
    // TODO wait until candidates are here

    m_session = CallSession::acceptCall(m_incomingSdp, m_incomingCandidates, m_cachedTurnUris, m_remoteUser->id(), this);
    m_participants->clear();
    connect(m_session.data(), &CallSession::stateChanged, this, [this] {
        Q_EMIT stateChanged();
        if (state() == CallSession::ICEFAILED) {
            Q_EMIT callEnded();
        }
    }); // TODO refactor away?
    m_incomingCandidates.clear();
    connectSingleShot(m_session.data(), &CallSession::answerCreated, this, [this](const QString &_sdp, const QVector<Candidate> &candidates) {
        const auto &[uuids, sdp] = mangleSdp(_sdp);
        QVector<std::pair<QString, QString>> msidToPurpose;
        for (const auto &uuid : uuids) {
            msidToPurpose += {uuid, "m.usermedia"}; // TODO
        }
        auto answer = createAnswer(m_callId, sdp, msidToPurpose);
        m_room->postJson("m.call.answer", answer);
        qCWarning(voip) << "Sending Answer";
        auto c = createCandidates(m_callId, candidates);
        auto cand = createCandidates(m_callId, candidates);
        m_room->postJson("m.call.candidates", cand);
        qCWarning(voip) << "Sending Candidates";
        setGlobalState(ACTIVE);
    });
}

void CallManager::hangupCall()
{
    qCDebug(voip) << "Ending call";
    if (m_session) {
        m_session->end();
        delete m_session;
        m_session = nullptr;
    }
    stopRinging();
    m_room->postJson("m.call.hangup", createHangup(m_callId));
    setGlobalState(IDLE);
    Q_EMIT callEnded();
}

NeoChatUser *CallManager::remoteUser() const
{
    return m_remoteUser;
}

NeoChatRoom *CallManager::room() const
{
    return m_room;
}

CallSession::State CallManager::state() const
{
    if (!m_session) {
        return CallSession::DISCONNECTED;
    }
    return m_session->state();
}

int CallManager::lifetime() const
{
    return m_lifetime;
}

void CallManager::ignoreCall()
{
    setLifetime(0);
    setCallId({});
    setRoom(nullptr);
    setRemoteUser(nullptr);
}

void CallManager::startCall(NeoChatRoom *room)
{
    if (m_session) {
        // Don't start calls if there already is one
        Q_EMIT Controller::instance().errorOccured(i18n("A call is already started"));
        return;
    }
    if (room->users().size() != 2) {
        // Don't start calls if the room doesn't have exactly two members
        Q_EMIT Controller::instance().errorOccured(i18n("Calls are limited to 1:1 rooms"));
        return;
    }

    auto missingPlugins = CallSession::missingPlugins();
    if (!missingPlugins.isEmpty()) {
        qCCritical(voip) << "Missing GStreamer plugins:" << missingPlugins;
        Q_EMIT Controller::instance().errorOccured("Missing GStreamer plugins.");
        return;
    }

    setLifetime(60000);
    setRoom(room);
    setRemoteUser(otherUser(room));

    updateTurnServers();

    setCallId(generateCallId());
    setPartyId(generatePartyId());

    m_participants->clear();
    for (const auto &user : m_room->users()) {
        auto participant = new CallParticipant(m_session);
        participant->m_user = dynamic_cast<NeoChatUser *>(user);
        m_participants->addParticipant(participant);
    }

    m_session = CallSession::startCall(m_cachedTurnUris, this);
    setGlobalState(OUTGOING);
    connect(m_session, &CallSession::stateChanged, this, [this] {
        Q_EMIT stateChanged();
        if (state() == CallSession::ICEFAILED) {
            Q_EMIT callEnded();
        }
    });

    connectSingleShot(m_session.data(), &CallSession::offerCreated, this, [this](const QString &_sdp, const QVector<Candidate> &candidates) {
        const auto &[uuids, sdp] = mangleSdp(_sdp);
        QVector<std::pair<QString, QString>> msidToPurpose;
        for (const auto &uuid : uuids) {
            msidToPurpose += {uuid, "m.usermedia"}; // TODO
        }
        qCWarning(voip) << "Sending Invite";
        qCWarning(voip) << "Sending Candidates";
        auto invite = createInvite(m_callId, sdp, msidToPurpose);
        auto c = createCandidates(m_callId, candidates);
        m_room->postJson("m.call.invite", invite);
        m_room->postJson("m.call.candidates", c);
    });

    connect(m_session, &CallSession::renegotiate, this, [this](const QString &sdp, const QString &type) {
        QVector<std::pair<QString, QString>> msidToPurpose;
        const auto &[uuids, _sdp] = mangleSdp(sdp);
        for (const auto &uuid : uuids) {
            msidToPurpose += {uuid, "m.usermedia"}; // TODO
        }
        QJsonObject json{
            {QStringLiteral("lifetime"), 60000},
            {QStringLiteral("version"), 1},
            {QStringLiteral("description"), QJsonObject{{QStringLiteral("type"), type}, {QStringLiteral("sdp"), _sdp}}}, // AAAAA
            {QStringLiteral("party_id"), m_partyId},
            {QStringLiteral("call_id"), m_callId},
        };
        QJsonObject metadata;
        for (const auto &[stream, purpose] : msidToPurpose) {
            QJsonObject data = {{"purpose", purpose}};
            metadata[stream] = data;
        }
        json["org.matrix.msc3077.sdp_stream_metadata"] = metadata;
        m_room->postJson("m.call.negotiate", json);
    });
}

QString CallManager::generateCallId() const
{
    return QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
}

QString CallManager::generatePartyId() const
{
    return QUuid::createUuid().toString();
}

void CallManager::setCallId(const QString &callId)
{
    m_callId = callId;
    Q_EMIT callIdChanged();
}

void CallManager::setPartyId(const QString &partyId)
{
    m_partyId = partyId;
}

void CallManager::setMuted(bool muted)
{
    if (!m_session) {
        return;
    }
    m_session->setMuted(muted);
    Q_EMIT mutedChanged();
}

bool CallManager::muted() const
{
    if (!m_session) {
        return false;
    }
    return m_session->muted();
}

bool CallManager::init()
{
    qRegisterMetaType<Candidate>();
    qRegisterMetaType<QVector<Candidate>>();
    GError *error = nullptr;
    if (!gst_init_check(nullptr, nullptr, &error)) {
        QString strError;
        if (error) {
            strError += error->message;
            g_error_free(error);
        }
        qCCritical(voip) << "Failed to initialize GStreamer:" << strError;
        return false;
    }

    gchar *version = gst_version_string();
    qCDebug(voip) << "GStreamer version" << version;
    g_free(version);

    // Required to register the qml types
    auto _sink = gst_element_factory_make("qmlglsink", nullptr);
    Q_ASSERT(_sink);
    gst_object_unref(_sink);
    return true;
}

void CallManager::setLifetime(int lifetime)
{
    m_lifetime = lifetime;
    Q_EMIT lifetimeChanged();
}

void CallManager::setRoom(NeoChatRoom *room)
{
    m_room = room;
    Q_EMIT roomChanged();
}

void CallManager::setRemoteUser(NeoChatUser *user)
{
    m_remoteUser = user;
    Q_EMIT roomChanged();
}

NeoChatUser *CallManager::otherUser(NeoChatRoom *room)
{
    return dynamic_cast<NeoChatUser *>(room->users()[0]->id() == room->localUser()->id() ? room->users()[1] : room->users()[0]);
}

QJsonObject CallManager::createCandidates(const QString &callId, const QVector<Candidate> &candidates) const
{
    QJsonArray candidatesJson;
    for (const auto &candidate : candidates) {
        candidatesJson += QJsonObject{{"candidate", candidate.candidate}, {"sdpMid", candidate.sdpMid}, {"sdpMLineIndex", candidate.sdpMLineIndex}};
    }
    return QJsonObject{{"call_id", callId}, {"candidates", candidatesJson}, {"version", CALL_VERSION}, {"party_id", "todopartyid"}};
}

void CallManager::setGlobalState(GlobalState globalState)
{
    if (m_globalState == globalState) {
        return;
    }
    m_globalState = globalState;
    Q_EMIT globalStateChanged();
}

CallManager::GlobalState CallManager::globalState() const
{
    return m_globalState;
}

CallParticipantsModel *CallManager::callParticipants() const
{
    return m_participants;
}

std::pair<QStringList, QString> CallManager::mangleSdp(const QString &_sdp)
{
    QString sdp = _sdp;
    QRegularExpression regex("msid:user[0-9]+@host-[0-9a-f]+ webrtctransceiver([0-9])");
    auto iter = regex.globalMatch(sdp);
    QStringList uuids;

    while (iter.hasNext()) {
        auto uuid = QUuid::createUuid();
        auto match = iter.next();
        uuids += uuid.toString();
        sdp.replace(match.captured(), QStringLiteral("msid:") + uuid.toString() + QStringLiteral(" foo"));
    }
    return {uuids, sdp};
}

QJsonObject CallManager::createInvite(const QString &callId, const QString &sdp, const QVector<std::pair<QString, QString>> &msidToPurpose) const
{
    QJsonObject metadata;
    for (const auto &[msid, purpose] : msidToPurpose) {
        metadata[msid] = QJsonObject{{"purpose", purpose}};
    }
    return {{"call_id", callId},
            {"party_id", "todopartyid"},
            {"lifetime", 60000},
            {"capabilities", QJsonObject{{"m.call.transferee", false}}},
            {"offer", QJsonObject{{"sdp", sdp}, {"type", "offer"}}},
            {"org.matrix.msc3077.sdp_stream_metadata", metadata},
            {"version", CALL_VERSION}};
}

QJsonObject CallManager::createHangup(const QString &callId) const
{
    return {{"call_id", callId}, {"party_id", "todopartyid"}, {"version", CALL_VERSION}};
}

QJsonObject CallManager::createAnswer(const QString &callId, const QString &sdp, const QVector<std::pair<QString, QString>> &msidToPurpose) const
{
    Q_ASSERT(!callId.isEmpty());
    QJsonObject metadata;
    for (const auto &[msid, purpose] : msidToPurpose) {
        metadata[msid] = QJsonObject{{"purpose", purpose}};
    }
    return {{"call_id", callId},
            {"party_id", "todopartyid"},
            {"lifetime", "lifetime"},
            {"capabilities", QJsonObject{{"m.call.transferee", false}}},
            {"offer", QJsonObject{{"sdp", sdp}, {"type", "offer"}}},
            {"org.matrix.msc3077.sdp_stream_metadata", metadata},
            {"version", CALL_VERSION}};
}

void CallManager::toggleCamera()
{
    m_session->toggleCamera();
}
QString CallManager::partyId() const
{
    return m_partyId;
}

bool CallManager::checkPlugins() const
{
    auto missingPlugins = m_session->missingPlugins();
    if (!missingPlugins.isEmpty()) {
        qCCritical(voip) << "Missing GStreamer plugins:" << missingPlugins;
        Q_EMIT Controller::instance().errorOccured("Missing GStreamer plugins.");
    }
    return !missingPlugins.isEmpty();
}
