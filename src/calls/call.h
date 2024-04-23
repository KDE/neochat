// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>

#include "neochatroom.h"

class Participant : public QObject
{
    Q_OBJECT

    void setVolume(float volume);
    void muteLocally();
    void unmuteLocally();

    void ring(); // See MSC4075

    // TODO: if these are possible; check livekit api
    void muteGlobally();
    void forceDisableCamera();
    void forceDisableScreenShare();

    void setPermissions();
    void kick();
    void ban();

Q_SIGNALS:
    void muted();
    void unmuted();

    void cameraEnabled();
    void cameraDisabled();

    void screenShareEnabled();
    void screenShareDisabled();
};

class Call : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    Q_PROPERTY(bool cameraEnabled READ cameraEnabled WRITE setCameraEnabled NOTIFY cameraEnabledChanged)
    Q_PROPERTY(bool microphoneMuted READ microphoneMuted WRITE setMicrophoneMuted NOTIFY microphoneMutedChanged)
    Q_PROPERTY(bool screenshareEnabled READ screenshareEnabled NOTIFY screenshareEnabledChanged)

    Q_PROPERTY(NeoChatRoom *room READ room CONSTANT)

public:
    explicit Call(NeoChatRoom *room, QObject *parent = nullptr);

Q_SIGNALS:
    void participantJoined(const Participant &participant);
    void participantLeft(const Participant &participant);

private:
    QList<Participant *> m_participants;
};

class CallParticipantsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRoleRole,
        HasCameraRole,
        HasScreenShareRole,
        IsMutedRole,
    };
    Q_ENUM(Roles)
};
