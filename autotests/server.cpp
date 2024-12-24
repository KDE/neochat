// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "server.h"

#include <QHttpServer>
#include <QSslServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHttpServerResponder>
#include <QNetworkReply>
#include <QJsonArray>
#include <QFile>
#include <QSslKey>
#include <QSslCertificate>

#include <Quotient/networkaccessmanager.h>

using namespace Qt::Literals::StringLiterals;

Server::Server()
{
}

void Server::start()
{
    QObject::connect(Quotient::NetworkAccessManager::instance(), &QNetworkAccessManager::sslErrors, Quotient::NetworkAccessManager::instance(), [](QNetworkReply *reply) {
        reply->ignoreSslErrors();
    });
    m_server.route(u"/.well-known/matrix/client"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(QJsonDocument(QJsonObject {
            {u"m.homeserver"_s, QJsonObject {{u"base_url"_s, u"https://localhost:1234"_s}}},
        }), QHttpServerResponder::StatusCode::Ok);
    });
    m_server.route(u"/_matrix/client/versions"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(QJsonDocument(QJsonObject {
            {u"versions"_s, QJsonArray { u"v1.0"_s, u"v1.1"_s, u"v1.2"_s, u"v1.3"_s, u"v1.4"_s, u"v1.5"_s, u"v1.6"_s, u"v1.7"_s, u"v1.8"_s, u"v1.9"_s, u"v1.10"_s, u"v1.11"_s, u"v1.12"_s, u"v1.13"_s,  }},
        }), QHttpServerResponder::StatusCode::Ok);
    });
    m_server.route(u"/_matrix/client/v3/capabilities"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(QJsonDocument(QJsonObject {
            {u"capabilities"_s, QJsonObject{
                {u"m.room_versions"_s, QJsonObject {
                    {u"m.available"_s, QJsonObject {
                        {u"1"_s, u"stable"_s}
                    }},
                    {u"default"_s, u"1"_s}
                }},
            }}
        }), QHttpServerResponder::StatusCode::Ok);
    });
    m_server.route(u"/_matrix/client/v3/account/whoami"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(QJsonDocument(QJsonObject {
            {u"device_id"_s, u"device_id_1234"_s},
            {u"user_id"_s, u"@user:localhost:1234"_s},
        }), QHttpServerResponder::StatusCode::Ok);
    });

    m_server.route(u"/_matrix/client/v3/login"_s, QHttpServerRequest::Method::Post, [](QHttpServerResponder &responder) {
        // TODO
        // if data["identifier"]["user"] != "user" or data["password"] != "1234":
        //     abort(403)
        responder.write(QJsonDocument(QJsonObject {
            {u"access_token"_s, u"token_login"_s},
            {u"device_id"_s, u"device_1234"_s},
            {u"user_id"_s, u"@user:localhost:1234"_s},
        }), QHttpServerResponder::StatusCode::Ok);
    });

    m_server.route(u"/_matrix/client/v3/login"_s, QHttpServerRequest::Method::Get, [](QHttpServerResponder &responder) {
        responder.write(QJsonDocument(QJsonObject {
            {u"flows"_s, QJsonArray {QJsonObject {
                {u"type"_s, u"m.login.password"_s}
            }
            }},
        }), QHttpServerResponder::StatusCode::Ok);
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
    }
}
