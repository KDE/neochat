// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later
#pragma once

#include <QHttpServer>
#include <QJsonObject>
#include <QSslServer>
#include <QtQml/QQmlEngine>
#include <QtQmlIntegration/qqmlintegration.h>

struct Changes {
    struct NewRoom {
        QStringList initialMembers;
        QString roomId;
        QStringList tags;
    };
    QList<NewRoom> newRooms;

    struct InviteUser {
        QString userId;
        QString roomId;
    };
    QList<InviteUser> invitations;

    struct BanUser {
        QString userId;
        QString roomId;
    };
    QList<BanUser> bans;

    struct JoinUser {
        QString userId;
        QString roomId;
    };
    QList<JoinUser> joins;

    struct Event {
        QJsonObject fullJson;
    };
    QList<Event> events;
    QList<Event> stateEvents;
};

struct RoomData {
    QStringList members;
    QString id;
    QStringList tags;
};

class Server : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static Server *instance()
    {
        static Server _instance;
        return &_instance;
    }
    static Server *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(instance(), QQmlEngine::CppOwnership);
        return instance();
    }

    void start();

    /**
     * Create a room and place the user with id matrixId in it.
     * Returns the room's id
     */
    Q_INVOKABLE QString createRoom(const QString &matrixId);

    void inviteUser(const QString &roomId, const QString &matrixId);
    void banUser(const QString &roomId, const QString &matrixId);
    void joinUser(const QString &roomId, const QString &matrixId);

    /**
     * Create a server notices room.
     */
    QString createServerNoticesRoom(const QString &matrixId);
    QString sendEvent(const QString &roomId, const QString &eventType, const QJsonObject &content);
    QString sendStateEvent(const QString &roomId, const QString &eventType, const QString &stateKey, const QJsonObject &content);

private:
    QHttpServer m_server;
    QSslServer m_sslServer;

    Server();
    void sync(const QHttpServerRequest &request, QHttpServerResponder &responder);

    QList<Changes> m_state;
};
