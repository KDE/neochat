// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <QHttpServer>
#include <QSslServer>

struct RoomData {
    QStringList members;
    QString id;
    QStringList tags;
};

class Server
{
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

    QHash<QString, QList<QString>> m_invitedUsers;
    QHash<QString, QList<QString>> m_bannedUsers;
    QHash<QString, QList<QString>> m_joinedUsers;

    QList<RoomData> m_roomsToCreate;
    QMap<QString, QJsonArray> m_events;
};
