// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "server.h"

#include <QFile>
#include <QHttpServerResponder>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QSslCertificate>
#include <QSslKey>
#include <QUuid>

#include <Quotient/networkaccessmanager.h>

using namespace Qt::Literals::StringLiterals;

QString generateEventId()
{
    return u"$"_s + QString::fromLatin1(QCryptographicHash::hash(QUuid::createUuid().toString().toLatin1(), QCryptographicHash::Sha1).toBase64());
}

QString generateRoomId()
{
    return u"!%1:localhost:1234"_s
        .arg(QString::fromLatin1(QCryptographicHash::hash(QUuid::createUuid().toString().toLatin1(), QCryptographicHash::Sha1).toBase64()))
        .replace(u'/', QChar());
}

Server::Server()
{
}

void Server::start()
{
    QObject::connect(Quotient::NetworkAccessManager::instance(),
                     &QNetworkAccessManager::sslErrors,
                     Quotient::NetworkAccessManager::instance(),
                     [](QNetworkReply *reply) {
                         reply->ignoreSslErrors();
                     });
    m_server.route(u"/.well-known/matrix/client"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(QJsonDocument(QJsonObject{
                            {u"m.homeserver"_s, QJsonObject{{u"base_url"_s, u"https://localhost:1234"_s}}},
                        }),
                        QHttpServerResponder::StatusCode::Ok);
    });
    m_server.route(u"/_matrix/client/versions"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(QJsonDocument(QJsonObject{
                            {u"versions"_s,
                             QJsonArray{
                                 u"v1.0"_s,
                                 u"v1.1"_s,
                                 u"v1.2"_s,
                                 u"v1.3"_s,
                                 u"v1.4"_s,
                                 u"v1.5"_s,
                                 u"v1.6"_s,
                                 u"v1.7"_s,
                                 u"v1.8"_s,
                                 u"v1.9"_s,
                                 u"v1.10"_s,
                                 u"v1.11"_s,
                                 u"v1.12"_s,
                                 u"v1.13"_s,
                             }},
                        }),
                        QHttpServerResponder::StatusCode::Ok);
    });
    m_server.route(u"/_matrix/client/v3/capabilities"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(
            QJsonDocument(QJsonObject{{u"capabilities"_s,
                                       QJsonObject{
                                           {u"m.room_versions"_s, QJsonObject{{u"m.available"_s, QJsonObject{{u"1"_s, u"stable"_s}}}, {u"default"_s, u"1"_s}}},
                                       }}}),
            QHttpServerResponder::StatusCode::Ok);
    });
    m_server.route(u"/_matrix/client/v3/account/whoami"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(QJsonDocument(QJsonObject{
                            {u"device_id"_s, u"device_id_1234"_s},
                            {u"user_id"_s, u"@user:localhost:1234"_s},
                        }),
                        QHttpServerResponder::StatusCode::Ok);
    });

    m_server.route(u"/_matrix/client/v3/login"_s, QHttpServerRequest::Method::Post, [](QHttpServerResponder &responder) {
        // TODO
        // if data["identifier"]["user"] != "user" or data["password"] != "1234":
        //     abort(403)
        responder.write(QJsonDocument(QJsonObject{
                            {u"access_token"_s, u"token_login"_s},
                            {u"device_id"_s, u"device_1234"_s},
                            {u"user_id"_s, u"@user:localhost:1234"_s},
                        }),
                        QHttpServerResponder::StatusCode::Ok);
    });

    m_server.route(u"/_matrix/client/v3/login"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(QJsonDocument(QJsonObject{
                            {u"flows"_s, QJsonArray{QJsonObject{{u"type"_s, u"m.login.password"_s}}}},
                        }),
                        QHttpServerResponder::StatusCode::Ok);
    });

    m_server.route(u"/_matrix/client/v3/rooms/<arg>/invite"_s,
                   QHttpServerRequest::Method::Post,
                   [this](const QString &roomId, QHttpServerResponder &responder, const QHttpServerRequest &request) {
                       Changes changes;
                       changes.invitations += Changes::InviteUser{
                           .userId = QJsonDocument::fromJson(request.body()).object()[u"user_id"_s].toString(),
                           .roomId = roomId,
                       };
                       m_state += changes;
                       responder.write(QJsonDocument(QJsonObject{}), QHttpServerResponder::StatusCode::Ok);
                   });

    m_server.route(u"/_matrix/client/r0/sync"_s, QHttpServerRequest::Method::Get, this, &Server::sync);

    QSslConfiguration config;
    QFile key(QStringLiteral(DATA_DIR) + u"/localhost.key"_s);
    void(key.open(QFile::ReadOnly));
    config.setPrivateKey(QSslKey(&key, QSsl::Rsa));
    config.setLocalCertificate(QSslCertificate::fromPath(QStringLiteral(DATA_DIR) + u"/localhost.crt"_s).front());
    m_sslServer.setSslConfiguration(config);
    if (!m_sslServer.listen(QHostAddress::LocalHost, 1234) || !m_server.bind(&m_sslServer)) {
        qFatal() << "Server failed to listen on a port.";
        return;
    } else {
        qWarning() << "Server listening";
    }
}

QString Server::createRoom(const QString &matrixId)
{
    const auto roomId = generateRoomId();
    Changes changes;
    changes.newRooms += Changes::NewRoom{
        .initialMembers = {matrixId},
        .roomId = {roomId},
        .tags = {},
    };
    m_state += changes;
    return roomId;
}

void Server::inviteUser(const QString &roomId, const QString &matrixId)
{
    Changes changes;
    changes.invitations += Changes::InviteUser{
        .userId = matrixId,
        .roomId = roomId,
    };
    m_state += changes;
}

void Server::banUser(const QString &roomId, const QString &matrixId)
{
    Changes changes;
    changes.bans += Changes::BanUser{
        .userId = matrixId,
        .roomId = roomId,
    };
    m_state += changes;
}

void Server::joinUser(const QString &roomId, const QString &matrixId)
{
    Changes changes;
    changes.joins += Changes::JoinUser{
        .userId = matrixId,
        .roomId = roomId,
    };
    m_state += changes;
}

QString Server::createServerNoticesRoom(const QString &matrixId)
{
    const auto roomId = generateRoomId();
    Changes changes;
    changes.newRooms += Changes::NewRoom{
        .initialMembers = {matrixId},
        .roomId = {roomId},
        .tags = {u"m.server_notice"_s},
    };
    m_state += changes;
    return roomId;
}

QString Server::sendEvent(const QString &roomId, const QString &eventType, const QJsonObject &content)
{
    Changes changes;
    const auto eventId = generateEventId();
    changes.events += Changes::Event{
        .fullJson = QJsonObject{{u"type"_s, eventType},
                                {u"content"_s, content},
                                {u"sender"_s, u"@foo:server.com"_s},
                                {u"event_id"_s, eventId},
                                {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                                {u"room_id"_s, roomId}},
    };
    m_state += changes;
    return eventId;
}

void Server::sync(const QHttpServerRequest &request, QHttpServerResponder &responder)
{
    QJsonObject joinRooms;
    auto token = request.query().queryItemValue(u"since"_s).toInt();

    for (const auto &change : m_state.mid(token)) {
        for (const auto &newRoom : change.newRooms) {
            QJsonArray stateEvents;
            stateEvents += QJsonObject{
                {u"content"_s, QJsonObject{{u"room_version"_s, u"11"_s}}},
                {u"event_id"_s, generateEventId()},
                {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                {u"room_id"_s, newRoom.roomId},
                {u"sender"_s, newRoom.initialMembers[0]},
                {u"state_key"_s, QString()},
                {u"type"_s, u"m.room.create"_s},
                {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
            };
            for (const auto &member : newRoom.initialMembers) {
                stateEvents += QJsonObject{
                    {u"content"_s, QJsonObject{{u"displayname"_s, u"User"_s}, {u"membership"_s, u"join"_s}}},
                    {u"event_id"_s, generateEventId()},
                    {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                    {u"room_id"_s, newRoom.roomId},
                    {u"sender"_s, member},
                    {u"state_key"_s, member},
                    {u"type"_s, u"m.room.member"_s},
                    {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
                };
            }

            auto room = QJsonObject{{u"state"_s, QJsonObject{{u"events"_s, stateEvents}}}};

            QJsonArray roomAccountData;
            QJsonObject tags;
            for (const auto &tag : newRoom.tags) {
                tags[tag] = QJsonObject();
            }
            if (!tags.empty()) {
                roomAccountData += QJsonObject{{u"type"_s, u"m.tag"_s}, {u"content"_s, QJsonObject{{u"tags"_s, tags}}}};
            }

            if (roomAccountData.size() > 0) {
                room[u"account_data"] = QJsonObject{{u"events"_s, roomAccountData}};
            }

            joinRooms[newRoom.roomId] = room;
        }
    }

    for (const auto &change : m_state.mid(token)) {
        for (const auto &invitation : change.invitations) {
            // TODO: The invitation could be for a room we haven't joined yet. Shouldn't be necessary for now, though.
            auto stateEvents = joinRooms[invitation.roomId][u"state"_s][u"events"_s].toArray();
            stateEvents += QJsonObject{
                {u"content"_s, QJsonObject{{u"displayname"_s, u"User"_s}, {u"membership"_s, u"invite"_s}}},
                {u"event_id"_s, generateEventId()},
                {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                {u"room_id"_s, invitation.roomId},
                {u"sender"_s, u"@user:localhost:1234"_s},
                {u"state_key"_s, invitation.userId},
                {u"type"_s, u"m.room.member"_s},
                {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
            };
            if (joinRooms.contains(invitation.roomId)) {
                auto room = joinRooms[invitation.roomId].toObject();
                room[u"state"_s] = QJsonObject{{u"events"_s, stateEvents}};
                joinRooms[invitation.roomId] = room;
            } else {
                joinRooms[invitation.roomId] = QJsonObject{{u"state"_s,
                                                            QJsonObject{
                                                                {u"events"_s, stateEvents},
                                                            }}};
            }
        }
    }

    for (const auto &change : m_state.mid(token)) {
        for (const auto &ban : change.bans) {
            // TODO: The ban could be for a room we haven't joined yet. Shouldn't be necessary for now, though.
            auto stateEvents = joinRooms[ban.roomId][u"state"_s][u"events"_s].toArray();
            stateEvents += QJsonObject{
                {u"content"_s, QJsonObject{{u"displayname"_s, u"User"_s}, {u"membership"_s, u"ban"_s}}},
                {u"event_id"_s, generateEventId()},
                {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                {u"room_id"_s, ban.roomId},
                {u"sender"_s, u"@user:localhost:1234"_s},
                {u"state_key"_s, ban.userId},
                {u"type"_s, u"m.room.member"_s},
                {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
            };
            if (joinRooms.contains(ban.roomId)) {
                auto room = joinRooms[ban.roomId].toObject();
                room[u"state"_s] = QJsonObject{{u"events"_s, stateEvents}};
                joinRooms[ban.roomId] = room;
            } else {
                joinRooms[ban.roomId] = QJsonObject{{u"state"_s,
                                                     QJsonObject{
                                                         {u"events"_s, stateEvents},
                                                     }}};
            }
        }
    }

    for (const auto &change : m_state.mid(token)) {
        for (const auto &join : change.joins) {
            // TODO: The join could be for a room we haven't joined yet. Shouldn't be necessary for now, though.
            auto stateEvents = joinRooms[join.roomId][u"state"_s][u"events"_s].toArray();
            stateEvents += QJsonObject{
                {u"content"_s, QJsonObject{{u"displayname"_s, u"User"_s}, {u"membership"_s, u"join"_s}}},
                {u"event_id"_s, generateEventId()},
                {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                {u"room_id"_s, join.roomId},
                {u"sender"_s, u"@user:localhost:1234"_s},
                {u"state_key"_s, join.userId},
                {u"type"_s, u"m.room.member"_s},
                {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
            };
            if (joinRooms.contains(join.roomId)) {
                auto room = joinRooms[join.roomId].toObject();
                room[u"state"_s] = QJsonObject{{u"events"_s, stateEvents}};
                joinRooms[join.roomId] = room;
            } else {
                joinRooms[join.roomId] = QJsonObject{{u"state"_s,
                                                      QJsonObject{
                                                          {u"events"_s, stateEvents},
                                                      }}};
            }
        }
    }

    for (const auto &change : m_state.mid(token)) {
        for (const auto &event : change.events) {
            // TODO the room might be in a different join state.
            auto timeline = joinRooms[event.fullJson[u"room_id"_s].toString()][u"timeline"_s][u"events"_s].toArray();
            timeline += event.fullJson;
            if (joinRooms.contains(event.fullJson[u"room_id"_s].toString())) {
                auto room = joinRooms[event.fullJson[u"room_id"_s].toString()].toObject();
                room[u"timeline"_s] = QJsonObject{{u"events"_s, timeline}};
                joinRooms[event.fullJson[u"room_id"_s].toString()] = room;
            } else {
                joinRooms[event.fullJson[u"room_id"_s].toString()] = QJsonObject{
                    {u"timeline"_s, QJsonObject{{u"events"_s, timeline}}},
                };
            }
        }
    }

    QJsonObject syncData = {
        // {u"account_data"_s, QJsonObject {}},
        // {u"presence"_s, QJsonObject {}},
        {u"next_batch"_s, QString::number(m_state.size())},
    };

    QJsonObject rooms;
    if (!joinRooms.isEmpty()) {
        rooms[u"join"_s] = joinRooms;
    }

    if (!rooms.empty()) {
        syncData[u"rooms"_s] = rooms;
    }

    qWarning() << syncData;
    responder.write(QJsonDocument(syncData), QHttpServerResponder::StatusCode::Ok);
}
