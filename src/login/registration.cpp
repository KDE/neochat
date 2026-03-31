// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "registration.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

#include <Quotient/qt_connection_util.h>
#include <Quotient/settings.h>

#include <KLocalizedString>

using namespace Quotient;

Registration::Registration()
{
    auto server = new QTcpServer(this);
    server->listen(QHostAddress("127.0.0.1"_L1), 20847);
    connect(server, &QTcpServer::newConnection, this, [this, server]() {
        auto conn = server->nextPendingConnection();
        connect(conn, &QIODevice::readyRead, this, [this, conn]() {
            auto code =
                "HTTP/1.0 200\nContent-type: text/html\n\n<html><head><script src=\"https://www.google.com/recaptcha/api.js\" async defer></script></head><body style=\"background: #00000000\"><center><div class=\"g-recaptcha\" data-sitekey=\"%1\"></div></center></body></html>"_L1
                    .arg(m_recaptchaSiteKey);
            conn->write(code.toLatin1().data(), code.length());
            conn->close();
        });
    });

    connect(this, &Registration::homeserverChanged, this, &Registration::testHomeserver);
    connect(this, &Registration::usernameChanged, this, &Registration::testUsername);
    connect(this, &Registration::registrationTokenChanged, this, &Registration::testRegistrationToken);
}

void Registration::setAccountManager(AccountManager *manager)
{
    m_accountManager = manager;
}

void Registration::setRecaptchaResponse(const QString &recaptchaResponse)
{
    m_recaptchaResponse = recaptchaResponse;
    Q_EMIT recaptchaResponseChanged();
    registerAccount();
}

QString Registration::recaptchaResponse() const
{
    return m_recaptchaResponse;
}

void Registration::setRecaptchaSiteKey(const QString &recaptchaSiteKey)
{
    m_recaptchaSiteKey = recaptchaSiteKey;
    Q_EMIT recaptchaSiteKeyChanged();
}

QString Registration::recaptchaSiteKey() const
{
    return m_recaptchaSiteKey;
}

void Registration::registerAccount()
{
    setStatus(Working);
    std::optional<QJsonObject> authData;
    if (nextStep() == "m.login.recaptcha"_L1) {
        authData = QJsonObject{
            {"type"_L1, "m.login.recaptcha"_L1},
            {"response"_L1, m_recaptchaResponse},
            {"session"_L1, m_session},
        };
    } else if (nextStep() == "m.login.registration_token"_L1) {
        authData = QJsonObject{
            {"type"_L1, "m.login.registration_token"_L1},
            {"token"_L1, m_registrationToken},
            {"session"_L1, m_session},
        };
    } else if (nextStep() == "m.login.terms"_L1) {
        authData = QJsonObject{
            {"type"_L1, "m.login.terms"_L1},
            {"session"_L1, m_session},
        };
    } else if (nextStep() == "m.login.email.identity"_L1) {
        authData = QJsonObject{
            {"type"_L1, "m.login.email.identity"_L1},
            {"threepid_creds"_L1,
             QJsonObject{
                 {"sid"_L1, m_sid},
                 {"client_secret"_L1, m_emailSecret},
             }},
            {"session"_L1, m_session},
        };
    }
    auto job = m_connection->callApi<NeoChatRegisterJob>("user"_L1, authData, m_username, m_password, QString(), QString(), true);
    connect(job, &BaseJob::result, this, [this, job]() {
        if (job->status() == BaseJob::Success) {
            setNextStep("loading"_L1);
            auto connection = new NeoChatConnection(this);
            auto matrixId = "@%1:%2"_L1.arg(m_username, m_homeserver);
            connection->resolveServer(matrixId);

            auto displayName = "NeoChat"_L1;
            connection->loginWithPassword(matrixId, m_password, displayName);

            connect(connection, &Connection::connected, this, [this, displayName, connection] {
                AccountSettings account(connection->userId());
                account.setKeepLoggedIn(true);
                account.setHomeserver(connection->homeserver());
                account.setDeviceId(connection->deviceId());
                account.setDeviceName(displayName);
                account.sync();
                m_accountManager->addConnection(connection);
                m_accountManager->setActiveConnection(connection);
                connect(
                    connection,
                    &Connection::syncDone,
                    this,
                    [this]() {
                        Q_EMIT loaded();
                    },
                    Qt::SingleShotConnection);
                m_connection = nullptr;
            });

            return;
        }

        if (job->status() == BaseJob::Unauthorised && nextStep() == "m.login.registration_token"_L1) {
            // Only reachable if validity endpoint is not implemented by homeserver.
            setStatus(InvalidRegistrationToken);
            return;
        }

        const auto &data = job->jsonData();
        m_session = data["session"_L1].toString();
        const auto &params = data["params"_L1].toObject();

        // I'm not motivated enough to figure out how we should handle the flow stuff, so:
        // If there is a flow that requires e-mail, we use that, to make sure that the user can recover the account from a forgotten password.
        // Otherwise, we're using the first flow.
        auto selectedFlow = data["flows"_L1].toArray()[0].toObject()["stages"_L1].toArray();
        for (const auto &flow : data["flows"_L1].toArray()) {
            if (flow.toObject()["stages"_L1].toArray().contains("m.login.email.identity"_L1)) {
                selectedFlow = flow.toObject()["stages"_L1].toArray();
            }
        }

        setNextStep(selectedFlow[data["completed"_L1].toArray().size()].toString());
        m_recaptchaSiteKey = params["m.login.recaptcha"_L1]["public_key"_L1].toString();
        Q_EMIT recaptchaSiteKeyChanged();
        m_terms.clear();
        for (const auto &term : params["m.login.terms"_L1]["policies"_L1].toObject().keys()) {
            QVariantMap termData;
            termData["title"_L1] = params["m.login.terms"_L1]["policies"_L1][term]["en"_L1]["name"_L1].toString();
            termData["url"_L1] = params["m.login.terms"_L1]["policies"_L1][term]["en"_L1]["url"_L1].toString();
            m_terms += termData;
            Q_EMIT termsChanged();
        }
    });
}

QString Registration::homeserver() const
{
    return m_homeserver;
}

void Registration::setHomeserver(const QString &url)
{
    m_homeserver = url;
    Q_EMIT homeserverChanged();
}

void Registration::testHomeserver()
{
    if (m_homeserver.isEmpty()) {
        setStatus(NoServer);
        return;
    }
    setStatus(TestingHomeserver);
    if (m_connection) {
        delete m_connection;
    }

    m_connection = new NeoChatConnection(this);
    m_connection->resolveServer("@user:%1"_L1.arg(m_homeserver));
    connect(
        m_connection.data(),
        &Connection::loginFlowsChanged,
        this,
        [this]() {
            if (m_testServerJob) {
                delete m_testServerJob;
            }
            auto ssoFlow = m_connection->getLoginFlow(LoginFlowTypes::SSO);
            if (ssoFlow && ssoFlow->delegatedOidcCompatibility) {
                auto session = m_connection->prepareForSso(u"NeoChat"_s);
                m_oidcUrl = session->ssoUrlForRegistration();
                Q_EMIT oidcUrlChanged();
                setStatus(Oidc);
                connect(m_connection, &Connection::connected, this, [this] {
                    Q_EMIT connected(m_connection.get());
                    Q_ASSERT(m_connection);
                    AccountSettings account(m_connection->userId());
                    account.setKeepLoggedIn(true);
                    account.setHomeserver(m_connection->homeserver());
                    account.setDeviceId(m_connection->deviceId());
                    account.sync();
                    QMetaObject::invokeMethod(
                        this,
                        [this]() {
                            m_accountManager->addConnection(m_connection);
                            m_accountManager->setActiveConnection(m_connection);
                            m_connection = nullptr;
                        },
                        Qt::QueuedConnection);
                    connect(
                        m_connection.get(),
                        &Connection::syncDone,
                        this,
                        [this]() {
                            Q_EMIT loaded();
                        },
                        Qt::SingleShotConnection);
                });
                return;
            }
            m_testServerJob = m_connection->callApi<NeoChatRegisterJob>("user"_L1, std::nullopt, "user"_L1, QString(), QString(), QString(), false);

            connect(m_testServerJob.data(), &BaseJob::finished, this, [this]() {
                if (m_testServerJob->error() == BaseJob::StatusCode::ContentAccessError) {
                    setStatus(ServerNoRegistration);
                    return;
                }
                if (m_testServerJob->status().code == BaseJob::StatusCode::Unauthorised) {
                    setStatus(RegistrationTokenRequired);
                    return;
                }
                if (m_testServerJob->status().code != 106) {
                    setStatus(InvalidServer);
                    return;
                }
                if (!m_username.isEmpty()) {
                    setStatus(TestingUsername);
                    testUsername();
                } else {
                    setStatus(NoUsername);
                }
            });
        },
        Qt::SingleShotConnection);
}

void Registration::setRegistrationToken(const QString &registrationToken)
{
    m_registrationToken = registrationToken;
    Q_EMIT registrationTokenChanged();
}

QString Registration::registrationToken() const
{
    return m_registrationToken;
}

void Registration::testRegistrationToken()
{
    if (status() <= ServerNoRegistration) {
        return;
    }

    setStatus(TestingRegistrationToken);

    if (m_testValidityJob) {
        m_testValidityJob->abandon();
    }
    if (m_registrationToken.isEmpty() || m_registrationToken.length() > 64) {
        setStatus(InvalidRegistrationToken);
        return;
    }

    m_testValidityJob = m_connection->callApi<RegistrationTokenValidityJob>(m_registrationToken).onResult([this]() {
        if (m_testValidityJob->error() == BaseJob::StatusCode::NotFound) {
            setStatus(NoRegistrationTokenPrevalidation);
        } else {
            setStatus(m_testValidityJob->error() == BaseJob::StatusCode::Success && m_testValidityJob->valid() ? Ready : InvalidRegistrationToken);
        }
    });
}

void Registration::setUsername(const QString &username)
{
    m_username = username;
    Q_EMIT usernameChanged();
}

QString Registration::username() const
{
    return m_username;
}

void Registration::testUsername()
{
    if (status() <= ServerNoRegistration) {
        return;
    }
    setStatus(TestingUsername);
    if (m_usernameJob) {
        m_usernameJob->abandon();
    }
    if (m_username.isEmpty()) {
        setStatus(NoUsername);
        return;
    }

    m_usernameJob = m_connection->callApi<CheckUsernameAvailabilityJob>(m_username);
    connect(m_usernameJob, &BaseJob::result, this, [this]() {
        setStatus(m_usernameJob->error() == BaseJob::StatusCode::Success && *m_usernameJob->available() ? Ready : UsernameTaken);
    });
}

QList<QVariantMap> Registration::terms() const
{
    return m_terms;
}

QString Registration::password() const
{
    return m_password;
}

void Registration::setPassword(const QString &password)
{
    m_password = password;
    Q_EMIT passwordChanged();
}

NeoChatRegisterJob::NeoChatRegisterJob(const QString &kind,
                                       const std::optional<QJsonObject> &auth,
                                       const QString &username,
                                       const QString &password,
                                       const QString &deviceId,
                                       const QString &initialDeviceDisplayName,
                                       std::optional<bool> inhibitLogin)
    : BaseJob(HttpVerb::Post, "RegisterJob"_L1, QByteArrayLiteral("/_matrix/client/r0/register"), false)
{
    QJsonObject _data;
    if (auth) {
        addParam<>(_data, "auth"_L1, auth);
    }
    addParam<>(_data, "username"_L1, username);
    addParam<IfNotEmpty>(_data, "password"_L1, password);
    addParam<IfNotEmpty>(_data, "device_id"_L1, deviceId);
    addParam<IfNotEmpty>(_data, "initial_device_display_name"_L1, initialDeviceDisplayName);
    addParam<IfNotEmpty>(_data, "inhibit_login"_L1, inhibitLogin);
    addParam<IfNotEmpty>(_data, "kind"_L1, kind);
    addParam<IfNotEmpty>(_data, "refresh_token"_L1, false);
    setRequestData(_data);
}

QString Registration::email() const
{
    return m_email;
}

void Registration::setEmail(const QString &email)
{
    m_email = email;
    Q_EMIT emailChanged();
}

QString Registration::nextStep() const
{
    return m_nextStep;
}

void Registration::setNextStep(const QString &nextStep)
{
    m_nextStep = nextStep;
    Q_EMIT nextStepChanged();
}

Registration::Status Registration::status() const
{
    return m_status;
}

QString Registration::statusString() const
{
    switch (m_status) {
    case NoServer:
        return i18nc("@info", "No server.");
    case TestingHomeserver:
        return i18nc("@info", "Checking Server availability.");
    case InvalidServer:
        return i18nc("@info", "This is not a valid server.");
    case ServerNoRegistration:
        return i18nc("@info", "Registration for this server is disabled.");
    case RegistrationTokenRequired:
        return i18nc("@info", "Registration for this server requires a registration token.");
    case NoRegistrationTokenPrevalidation:
        return i18nc("@info", "This server cannot prevalidate registration tokens.");
    case InvalidRegistrationToken:
        return i18nc("@info", "This is not a valid registration token.");
    case NoUsername:
        return i18nc("@info", "No username.");
    case TestingUsername:
        return i18nc("@info", "Checking username availability.");
    case UsernameTaken:
        return i18nc("@info", "This username is not available.");
    case Ready:
        return i18nc("@info", "Continue");
    case Working:
        return i18nc("@info", "Working");
    case TestingRegistrationToken:
        return i18nc("@info", "Checking registration token.");
    case Oidc:
        return i18nc("@info", "Waiting for login confirmation in your browser.");
    }
    return {};
}

void Registration::setStatus(Status status)
{
    m_status = status;
    Q_EMIT statusChanged();
}

void Registration::registerEmail()
{
    m_emailSecret = QString::fromLatin1(QUuid::createUuid().toString().toLatin1().toBase64());
    EmailValidationData data;
    data.email = m_email;
    data.clientSecret = m_emailSecret;
    data.sendAttempt = 0;

    auto job = m_connection->callApi<RequestTokenToRegisterEmailJob>(data);
    connect(job, &BaseJob::finished, this, [this, job]() {
        m_sid = job->jsonData()["sid"_L1].toString();
    });
}

#include "moc_registration.cpp"
