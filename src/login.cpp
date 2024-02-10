// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "login.h"

#include <Quotient/accountregistry.h>
#include <Quotient/connection.h>
#include <Quotient/jobs/basejob.h>
#include <Quotient/qt_connection_util.h>

#include "controller.h"

#include <KLocalizedString>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTcpServer>

using namespace Quotient;

LoginHelper::LoginHelper(QObject *parent)
    : QObject(parent)
{
    init();
}

void LoginHelper::init()
{
    m_homeserverReachable = false;
    m_connection = new NeoChatConnection();
    m_matrixId = QString();
    m_password = QString();
    m_deviceName = QStringLiteral("NeoChat %1 %2 %3 %4")
                       .arg(QSysInfo::machineHostName(), QSysInfo::productType(), QSysInfo::productVersion(), QSysInfo::currentCpuArchitecture());
    m_supportsSso = false;
    m_supportsPassword = false;
    m_ssoUrl = QUrl();

    connect(this, &LoginHelper::matrixIdChanged, this, [this]() {
        setHomeserverReachable(false);
        QRegularExpression validator(QStringLiteral("^\\@?[a-zA-Z0-9\\._=\\-/]+\\:[a-zA-Z0-9\\-]+(\\.[a-zA-Z0-9\\-]+)*(\\:[0-9]+)?$"));
        if (!validator.match(m_matrixId).hasMatch()) {
            return;
        }

        if (m_matrixId == QLatin1Char('@')) {
            return;
        }

        m_isLoggedIn = Controller::instance().accounts().isLoggedIn(m_matrixId);
        Q_EMIT isLoggedInChanged();
        if (m_isLoggedIn) {
            return;
        }

        m_testing = true;
        Q_EMIT testingChanged();
        if (!m_connection) {
            m_connection = new NeoChatConnection();
        }
        m_connection->resolveServer(m_matrixId);
        connectSingleShot(m_connection, &Connection::loginFlowsChanged, this, [this]() {
            setHomeserverReachable(true);
            m_testing = false;
            Q_EMIT testingChanged();
            static QNetworkAccessManager *nam;
            if (!nam) {
                nam = new QNetworkAccessManager(this);
                auto configUrl = QStringLiteral("%1.well-known/openid-configuration").arg(m_connection->authIssuer());
                QNetworkRequest request((QUrl(configUrl)));
                auto reply = nam->get(request);

                connect(reply, &QNetworkReply::finished, this, [this, reply] {
                    auto configurationJson = QJsonDocument::fromJson(reply->readAll()).object();

                    QJsonObject metadata{
                        {"client_name"_ls, "NeoChat"_ls},
                        {"client_uri"_ls, "https://apps.kde.org/neochat"_ls},
                        {"logo_uri"_ls, "https://apps.kde.org/app-icons/org.kde.neochat.svg"_ls},
                        {"contacts"_ls, QJsonArray{"tobias.fella@kde.org"_ls}},
                        {"tos_uri"_ls, "https://apps.kde.org/neochat"_ls},
                        {"policy_uri"_ls, "https://apps.kde.org/neochat"_ls},
                        {"redirect_uris"_ls, QJsonArray{"http://127.0.0.1:21431/redirect"_ls}},
                        {"application_type"_ls, "native"_ls},
                    };
                    QNetworkRequest request(QUrl(configurationJson["registration_endpoint"_ls].toString()));
                    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"_ls);
                    auto reply = nam->post(request, QJsonDocument(metadata).toJson());
                    connect(reply, &QNetworkReply::finished, this, [reply, configurationJson, this] {
                        auto json = QJsonDocument::fromJson(reply->readAll()).object();
                        auto authorizationEndpoint = configurationJson["authorization_endpoint"_ls].toString();
                        QUrl authUrl(authorizationEndpoint);
                        QUrlQuery query;
                        // query.addQueryItem(QLatin1String("code_challenge"), challengeString);
                        // query.addQueryItem(QStringLiteral("code_challenge_method"), QLatin1String("S256"));
                        query.addQueryItem(QStringLiteral("client_id"), json["client_id"_ls].toString());
                        query.addQueryItem(QStringLiteral("redirect_uri"), QLatin1String("http://127.0.0.1:21431/redirect"));
                        query.addQueryItem(QStringLiteral("response_type"), QLatin1String("code"));
                        query.addQueryItem(QStringLiteral("scope"), QLatin1String("device:ABCDEF api:*"));
                        // query.addQueryItem(QLatin1String("state"), m_state);
                        authUrl.setQuery(query);
                        qWarning() << "open" << authUrl.toString();

                        auto server = new QTcpServer(this);
                        server->listen(QHostAddress(QStringLiteral("127.0.0.1")), 21431);
                        connect(server, &QTcpServer::newConnection, this, [this, server]() {
                            auto connection = server->nextPendingConnection();
                            connect(connection, &QIODevice::readyRead, this, [this, connection]() {
                                auto data = QString::fromLatin1(connection->readAll());
                                qWarning() << data;
                                // QRegularExpression codeRegex(QStringLiteral("code=([a-f0-9]+)&"));
                                // auto code = codeRegex.match(data).captured(1);
                                // QRegularExpression stateRegex(QStringLiteral("state=([0-9]+)"));
                                connection->write("HTTP/1.0 200 OK\r\n\r\nYou can return to NeoChat now.");
                                connection->close();
                                // if (stateRegex.match(data).captured(1) != m_state) {
                                //     return;
                                // }
                                // auto tokenUrl = buildUrl(QStringLiteral("/oauth/token"));
                                // QUrlQuery query;
                                // query.addQueryItem(QLatin1String("grant_type"), QStringLiteral("authorization_code"));
                                // query.addQueryItem(QLatin1String("code"), QString::fromLatin1(QUrl::toPercentEncoding(code)));
                                // query.addQueryItem(QLatin1String("redirect_uri"),
                                // QString::fromLatin1(QUrl::toPercentEncoding(QStringLiteral("http://127.0.0.1:11450"))));
                                // query.addQueryItem(QLatin1String("code_verifier"), QString::fromLatin1(m_verifier));
                                // query.addQueryItem(QLatin1String("client_id"), QLatin1String("98"));
                                // QNetworkRequest request(tokenUrl);
                                // request.setHeader(QNetworkRequest::ContentTypeHeader,
                                //     QStringLiteral("application/x-www-form-urlencoded"));
                                // auto reply = nam->post(request, query.toString(QUrl::FullyEncoded).toUtf8());
                                // connect(reply, &QNetworkReply::finished, this, [this, reply](){
                                //     auto json = QJsonDocument::fromJson(reply->readAll()).object();
                                //     setAccessToken(json[QLatin1String("access_token")].toString());
                                //     const auto refreshToken = json[QLatin1String("refresh_token")].toString();
                                //     writeKeychain(QStringLiteral("traewelling-access"), m_accessToken);
                                //     writeKeychain(QStringLiteral("traewelling-refresh"), refreshToken);
                                //     loadData();
                                // });
                            });
                        });

                        // auto reply = nam->post(request, QJsonDocument(authorizationData).toJson());
                    });
                });
            }
            Q_EMIT loginFlowsChanged();
        });
    });
    connect(m_connection, &Connection::connected, this, [this] {
        Q_EMIT connected();
        m_isLoggingIn = false;
        Q_EMIT isLoggingInChanged();
        Q_ASSERT(m_connection);
        AccountSettings account(m_connection->userId());
        account.setKeepLoggedIn(true);
        account.setHomeserver(m_connection->homeserver());
        account.setDeviceId(m_connection->deviceId());
        account.setDeviceName(m_deviceName);
        if (!Controller::instance().saveAccessTokenToKeyChain(account, m_connection->accessToken())) {
            qWarning() << "Couldn't save access token";
        }
        account.sync();
        Controller::instance().addConnection(m_connection);
        Controller::instance().setActiveConnection(m_connection);
        m_connection = nullptr;
    });
    connect(m_connection, &Connection::networkError, this, [this](QString error, const QString &, int, int) {
        Q_EMIT Controller::instance().errorOccured(i18n("Network Error"), std::move(error));
        m_isLoggingIn = false;
        Q_EMIT isLoggingInChanged();
    });
    connect(m_connection, &Connection::loginError, this, [this](QString error, const QString &) {
        if (error == QStringLiteral("Invalid username or password")) {
            setInvalidPassword(true);
        } else {
            Q_EMIT errorOccured(i18n("Login Failed: %1", error));
        }
        m_isLoggingIn = false;
        Q_EMIT isLoggingInChanged();
    });

    connect(m_connection, &Connection::resolveError, this, [](QString error) {
        Q_EMIT Controller::instance().errorOccured(i18n("Network Error"), std::move(error));
    });

    connectSingleShot(m_connection, &Connection::syncDone, this, [this]() {
        Q_EMIT loaded();
    });
}

void LoginHelper::setHomeserverReachable(bool reachable)
{
    m_homeserverReachable = reachable;
    Q_EMIT homeserverReachableChanged();
}

bool LoginHelper::homeserverReachable() const
{
    return m_homeserverReachable;
}

QString LoginHelper::matrixId() const
{
    return m_matrixId;
}

void LoginHelper::setMatrixId(const QString &matrixId)
{
    m_matrixId = matrixId;
    if (!m_matrixId.startsWith(QLatin1Char('@'))) {
        m_matrixId.prepend(QLatin1Char('@'));
    }
    Q_EMIT matrixIdChanged();
}

QString LoginHelper::password() const
{
    return m_password;
}

void LoginHelper::setPassword(const QString &password)
{
    setInvalidPassword(false);
    m_password = password;
    Q_EMIT passwordChanged();
}

QString LoginHelper::deviceName() const
{
    return m_deviceName;
}

void LoginHelper::setDeviceName(const QString &deviceName)
{
    m_deviceName = deviceName;
    Q_EMIT deviceNameChanged();
}

void LoginHelper::login()
{
    m_isLoggingIn = true;
    Q_EMIT isLoggingInChanged();

    // Some servers do not have a .well_known file. So we login via the username part from the mxid,
    // rather than with the full mxid, as that would lead to an invalid user.
    auto username = m_matrixId.mid(1, m_matrixId.indexOf(QLatin1Char(':')) - 1);
    m_connection->loginWithPassword(username, m_password, m_deviceName, QString());
}

bool LoginHelper::supportsPassword() const
{
    return m_supportsPassword;
}

bool LoginHelper::supportsSso() const
{
    return m_supportsSso;
}

QUrl LoginHelper::ssoUrl() const
{
    return m_ssoUrl;
}

void LoginHelper::loginWithSso()
{
    m_connection->resolveServer(m_matrixId);
    connectSingleShot(m_connection, &Connection::loginFlowsChanged, this, [this]() {
        SsoSession *session = m_connection->prepareForSso(m_deviceName);
        m_ssoUrl = session->ssoUrl();
        Q_EMIT ssoUrlChanged();
    });
}

bool LoginHelper::testing() const
{
    return m_testing;
}

bool LoginHelper::isLoggingIn() const
{
    return m_isLoggingIn;
}

bool LoginHelper::isLoggedIn() const
{
    return m_isLoggedIn;
}

void LoginHelper::setInvalidPassword(bool invalid)
{
    m_invalidPassword = invalid;
    Q_EMIT isInvalidPasswordChanged();
}

bool LoginHelper::isInvalidPassword() const
{
    return m_invalidPassword;
}

#include "moc_login.cpp"
