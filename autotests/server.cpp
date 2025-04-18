// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "server.h"

#include <QFile>
#include <QHttpServer>
#include <QHttpServerResponder>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslServer>
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
                       m_invitedUsers[roomId] += QJsonDocument::fromJson(request.body()).object()[u"user_id"_s].toString();
                       responder.write(QJsonDocument(QJsonObject{}), QHttpServerResponder::StatusCode::Ok);
                   });

    m_server.route(u"/_matrix/client/r0/sync"_s, QHttpServerRequest::Method::Get, [this](QHttpServerResponder &responder) {
        QMap<QString, QJsonArray> stateEvents;

        for (const auto &[roomId, matrixId] : m_roomsToCreate) {
            stateEvents[roomId] += QJsonObject{
                {u"content"_s, QJsonObject{{u"room_version"_s, u"11"_s}}},
                {u"event_id"_s, generateEventId()},
                {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                {u"room_id"_s, roomId},
                {u"sender"_s, matrixId},
                {u"state_key"_s, QString()},
                {u"type"_s, u"m.room.create"_s},
                {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
            };
            stateEvents[roomId] += QJsonObject{
                {u"content"_s, QJsonObject{{u"displayname"_s, u"User"_s}, {u"membership"_s, u"join"_s}}},
                {u"event_id"_s, generateEventId()},
                {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                {u"room_id"_s, roomId},
                {u"sender"_s, matrixId},
                {u"state_key"_s, matrixId},
                {u"type"_s, u"m.room.member"_s},
                {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
            };
        }
        m_roomsToCreate.clear();
        for (const auto &roomId : m_invitedUsers.keys()) {
            const auto &values = m_invitedUsers[roomId];
            for (const auto &value : values) {
                stateEvents[roomId] += QJsonObject{
                    {u"content"_s, QJsonObject{{u"displayname"_s, u"User"_s}, {u"membership"_s, u"invite"_s}}},
                    {u"event_id"_s, generateEventId()},
                    {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                    {u"room_id"_s, roomId},
                    {u"sender"_s, u"@user:localhost:1234"_s},
                    {u"state_key"_s, value},
                    {u"type"_s, u"m.room.member"_s},
                    {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
                };
            }
        }
        m_invitedUsers.clear();

        for (const auto &roomId : m_bannedUsers.keys()) {
            const auto &values = m_bannedUsers[roomId];
            for (const auto &value : values) {
                stateEvents[roomId] += QJsonObject{
                    {u"content"_s, QJsonObject{{u"displayname"_s, u"User"_s}, {u"membership"_s, u"ban"_s}}},
                    {u"event_id"_s, generateEventId()},
                    {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                    {u"room_id"_s, roomId},
                    {u"sender"_s, u"@user:localhost:1234"_s},
                    {u"state_key"_s, value},
                    {u"type"_s, u"m.room.member"_s},
                    {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
                };
            }
        }
        m_bannedUsers.clear();

        for (const auto &roomId : m_joinedUsers.keys()) {
            const auto &values = m_joinedUsers[roomId];
            for (const auto &value : values) {
                stateEvents[roomId] += QJsonObject{
                    {u"content"_s, QJsonObject{{u"displayname"_s, u"User"_s}, {u"membership"_s, u"join"_s}}},
                    {u"event_id"_s, generateEventId()},
                    {u"origin_server_ts"_s, QDateTime::currentMSecsSinceEpoch()},
                    {u"room_id"_s, roomId},
                    {u"sender"_s, u"@user:localhost:1234"_s},
                    {u"state_key"_s, value},
                    {u"type"_s, u"m.room.member"_s},
                    {u"unsigned"_s, QJsonObject{{u"age"_s, 1234}}},
                };
            }
        }
        m_joinedUsers.clear();

        QJsonObject rooms;
        for (const auto &roomId : stateEvents.keys()) {
            rooms[roomId] = QJsonObject{{u"state"_s, QJsonObject{{u"events"_s, stateEvents[roomId]}}}};
        }

        responder.write(QJsonDocument(QJsonObject{{u"rooms"_s, QJsonObject{{u"join"_s, rooms}}}}), QHttpServerResponder::StatusCode::Ok);
    });

    QSslConfiguration config;
    QFile key(QStringLiteral(DATA_DIR) + u"/localhost.key"_s);
    key.open(QFile::ReadOnly);
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
    auto roomId = generateRoomId();
    m_roomsToCreate += {roomId, matrixId};
    return roomId;
}

void Server::inviteUser(const QString &roomId, const QString &matrixId)
{
    m_invitedUsers[roomId] += matrixId;
}

void Server::banUser(const QString &roomId, const QString &matrixId)
{
    m_bannedUsers[roomId] += matrixId;
}

void Server::joinUser(const QString &roomId, const QString &matrixId)
{
    m_joinedUsers[roomId] += matrixId;
}
