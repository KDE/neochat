// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <QHttpServer>
#include <QJsonObject>
#include <QSslServer>

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
};

struct RoomData {
    QStringList members;
    QString id;
    QStringList tags;
};

class Server : public QObject
{
    Q_OBJECT

public:
    Server();

    void start();

    /**
     * Create a room and place the user with id matrixId in it.
     * Returns the room's id
     */
    QString createRoom(const QString &matrixId);

    void inviteUser(const QString &roomId, const QString &matrixId);
    void banUser(const QString &roomId, const QString &matrixId);
    void joinUser(const QString &roomId, const QString &matrixId);

    /**
     * Create a server notices room.
     */
    QString createServerNoticesRoom(const QString &matrixId);
    QString sendEvent(const QString &roomId, const QString &eventType, const QJsonObject &content);

private:
    QHttpServer m_server;
    QSslServer m_sslServer;

    void sync(const QHttpServerRequest &request, QHttpServerResponder &responder);

    QList<Changes> m_state;
};
