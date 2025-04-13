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
            m_testServerJob = m_connection->callApi<NeoChatRegisterJob>("user"_L1, std::nullopt, "user"_L1, QString(), QString(), QString(), false);

            connect(m_testServerJob.data(), &BaseJob::finished, this, [this]() {
                if (m_testServerJob->error() == BaseJob::StatusCode::ContentAccessError) {
                    setStatus(ServerNoRegistration);
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
        return i18n("No server.");
    case TestingHomeserver:
        return i18n("Checking Server availability.");
    case InvalidServer:
        return i18n("This is not a valid server.");
    case ServerNoRegistration:
        return i18n("Registration for this server is disabled.");
    case NoUsername:
        return i18n("No username.");
    case TestingUsername:
        return i18n("Checking username availability.");
    case UsernameTaken:
        return i18n("This username is not available.");
    case Ready:
        return i18n("Continue");
    case Working:
        return i18n("Working");
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
