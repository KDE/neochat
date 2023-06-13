// SPDX-FileCopyrightText: 2020-2021 Nheko Authors
// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "neochatroom.h"
#include "neochatuser.h"
#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <events/roomevent.h>

#include "callsession.h"

#include "models/callparticipantsmodel.h"
#include <events/callevents.h>

#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QTimer>
#include <qcoro/task.h>

#include <qobjectdefs.h>

class CallSession;
class QQuickItem;

using namespace Quotient;

class CallManager : public QObject
{
    Q_OBJECT

public:
    enum GlobalState {
        IDLE,
        INCOMING,
        OUTGOING,
        ACTIVE,
    };
    Q_ENUM(GlobalState);

    Q_PROPERTY(GlobalState globalState READ globalState NOTIFY globalStateChanged)
    Q_PROPERTY(NeoChatUser *remoteUser READ remoteUser NOTIFY remoteUserChanged)
    Q_PROPERTY(QString callId READ callId NOTIFY callIdChanged)
    Q_PROPERTY(NeoChatRoom *room READ room NOTIFY roomChanged)
    Q_PROPERTY(int lifetime READ lifetime NOTIFY lifetimeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(QQuickItem *item MEMBER m_item) // TODO allow for different devices for each session
    Q_PROPERTY(CallSession::State state READ state NOTIFY stateChanged)
    Q_PROPERTY(CallParticipantsModel *callParticipants READ callParticipants CONSTANT)

    static CallManager &instance()
    {
        static CallManager _instance;
        return _instance;
    }

    [[nodiscard]] QString callId() const;
    [[nodiscard]] QString partyId() const;

    CallSession::State state() const;

    NeoChatUser *remoteUser() const;
    NeoChatRoom *room() const;

    int lifetime() const;

    bool muted() const;
    void setMuted(bool muted);

    CallManager::GlobalState globalState() const;

    void handleCallEvent(NeoChatRoom *room, const RoomEvent *event);

    Q_INVOKABLE void startCall(NeoChatRoom *room);
    Q_INVOKABLE void acceptCall();
    Q_INVOKABLE void hangupCall();
    Q_INVOKABLE void ignoreCall();

    Q_INVOKABLE void toggleCamera();

    QCoro::Task<void> updateTurnServers();

    [[nodiscard]] CallParticipantsModel *callParticipants() const;

    QQuickItem *m_item = nullptr;

Q_SIGNALS:
    void currentCallIdChanged();
    void incomingCall(NeoChatUser *user, NeoChatRoom *room, int timeout, const QString &callId);
    void callEnded();
    void remoteUserChanged();
    void callIdChanged();
    void roomChanged();
    void stateChanged();
    void lifetimeChanged();
    void mutedChanged();
    void globalStateChanged();

private:
    CallManager();
    QString m_callId;

    QVector<Candidate> m_incomingCandidates;
    QString m_incomingSdp;

    [[nodiscard]] bool checkPlugins() const;

    QStringList m_cachedTurnUris;
    QDateTime m_cachedTurnUrisValidUntil = QDateTime::fromSecsSinceEpoch(0);

    NeoChatUser *m_remoteUser = nullptr;
    NeoChatRoom *m_room = nullptr;
    QString m_remotePartyId;
    QString m_partyId;
    int m_lifetime = 0;

    GlobalState m_globalState = IDLE;

    void handleInvite(NeoChatRoom *room, const CallInviteEvent *event);
    void handleHangup(NeoChatRoom *room, const CallHangupEvent *event);
    void handleCandidates(NeoChatRoom *room, const CallCandidatesEvent *event);
    void handleAnswer(NeoChatRoom *room, const CallAnswerEvent *event);
    void handleNegotiate(NeoChatRoom *room, const CallNegotiateEvent *event);
    void checkStartCall();

    void ring(int lifetime);
    void stopRinging();

    [[nodiscard]] QString generateCallId() const;
    [[nodiscard]] QString generatePartyId() const;
    bool init();

    bool m_initialised = false;
    QPointer<CallSession> m_session = nullptr;

    void setLifetime(int lifetime);
    void setRoom(NeoChatRoom *room);
    void setRemoteUser(NeoChatUser *user);
    void setCallId(const QString &callId);
    void setPartyId(const QString &partyId);
    void setGlobalState(GlobalState state);

    std::pair<QStringList, QString> mangleSdp(const QString &sdp);

    CallParticipantsModel *m_participants = new CallParticipantsModel();

    NeoChatUser *otherUser(NeoChatRoom *room);

    [[nodiscard]] QJsonObject createCandidates(const QString &callId, const QVector<Candidate> &candidates) const;
    [[nodiscard]] QJsonObject createInvite(const QString &callId, const QString &sdp, const QVector<std::pair<QString, QString>> &msidToPurpose) const;
    [[nodiscard]] QJsonObject createHangup(const QString &callId) const;
    [[nodiscard]] QJsonObject createAnswer(const QString &callId, const QString &sdp, const QVector<std::pair<QString, QString>> &msidToPurpose) const;

    QMediaPlayer m_ringPlayer;
    QMediaPlaylist m_playlist;
};
