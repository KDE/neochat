// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "registration.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

#include <Quotient/csapi/registration.h>
#include <Quotient/qt_connection_util.h>
#include <Quotient/settings.h>

#include "controller.h"

#include <KLocalizedString>

using namespace Quotient;

Registration::Registration()
{
    auto server = new QTcpServer(this);
    server->listen(QHostAddress("127.0.0.1"_ls), 20847);
    connect(server, &QTcpServer::newConnection, this, [=]() {
        auto conn = server->nextPendingConnection();
        connect(conn, &QIODevice::readyRead, this, [=]() {
            auto code =
                "HTTP/1.0 200\nContent-type: text/html\n\n<html><head><script src=\"https://www.google.com/recaptcha/api.js\" async defer></script></head><body style=\"background: #00000000\"><center><div class=\"g-recaptcha\" data-sitekey=\"%1\"></div></center></body></html>"_ls
                    .arg(m_recaptchaSiteKey);
            conn->write(code.toLatin1().data(), code.length());
            conn->close();
        });
    });

    connect(this, &Registration::homeserverChanged, this, &Registration::testHomeserver);
    connect(this, &Registration::usernameChanged, this, &Registration::testUsername);
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
    Omittable<QJsonObject> authData = none;
    if (nextStep() == "m.login.recaptcha"_ls) {
        authData = QJsonObject{
            {"type"_ls, "m.login.recaptcha"_ls},
            {"response"_ls, m_recaptchaResponse},
            {"session"_ls, m_session},
        };
    } else if (nextStep() == "m.login.terms"_ls) {
        authData = QJsonObject{
            {"type"_ls, "m.login.terms"_ls},
            {"session"_ls, m_session},
        };
    } else if (nextStep() == "m.login.email.identity"_ls) {
        authData = QJsonObject{
            {"type"_ls, "m.login.email.identity"_ls},
            {"threepid_creds"_ls,
             QJsonObject{
                 {"sid"_ls, m_sid},
                 {"client_secret"_ls, m_emailSecret},
             }},
            {"session"_ls, m_session},
        };
    }
    auto job = m_connection->callApi<NeoChatRegisterJob>("user"_ls, authData, m_username, m_password, QString(), QString(), true);
    connect(job, &BaseJob::result, this, [=]() {
        if (job->status() == BaseJob::Success) {
            setNextStep("loading"_ls);
            auto connection = new Connection(this);
            auto matrixId = "@%1:%2"_ls.arg(m_username, m_homeserver);
            connection->resolveServer(matrixId);

            auto displayName = "NeoChat %1 %2 %3 %4"_ls.arg(QSysInfo::machineHostName(),
                                                            QSysInfo::productType(),
                                                            QSysInfo::productVersion(),
                                                            QSysInfo::currentCpuArchitecture());
            connection->loginWithPassword(matrixId, m_password, displayName);

            connect(connection, &Connection::connected, this, [this, displayName, connection] {
                AccountSettings account(connection->userId());
                account.setKeepLoggedIn(true);
                account.setHomeserver(connection->homeserver());
                account.setDeviceId(connection->deviceId());
                account.setDeviceName(displayName);
                if (!Controller::instance().saveAccessTokenToKeyChain(account, connection->accessToken())) {
                    qWarning() << "Couldn't save access token";
                }
                account.sync();
                Controller::instance().addConnection(connection);
                Controller::instance().setActiveConnection(connection);
                connectSingleShot(connection, &Connection::syncDone, this, []() {
                    Q_EMIT Controller::instance().initiated();
                });
                m_connection = nullptr;
            });

            return;
        }
        const auto &data = job->jsonData();
        m_session = data["session"_ls].toString();
        const auto &params = data["params"_ls].toObject();

        // I'm not motivated enough to figure out how we should handle the flow stuff, so:
        // If there is a flow that requires e-mail, we use that, to make sure that the user can recover the account from a forgotten password.
        // Otherwise, we're using the first flow.
        auto selectedFlow = data["flows"_ls].toArray()[0].toObject()["stages"_ls].toArray();
        for (const auto &flow : data["flows"_ls].toArray()) {
            if (flow.toObject()["stages"_ls].toArray().contains("m.login.email.identity"_ls)) {
                selectedFlow = flow.toObject()["stages"_ls].toArray();
            }
        }

        setNextStep(selectedFlow[data["completed"_ls].toArray().size()].toString());
        m_recaptchaSiteKey = params["m.login.recaptcha"_ls]["public_key"_ls].toString();
        Q_EMIT recaptchaSiteKeyChanged();
        m_terms.clear();
        for (const auto &term : params["m.login.terms"_ls]["policies"_ls].toObject().keys()) {
            QVariantMap termData;
            termData["title"_ls] = params["m.login.terms"_ls]["policies"_ls][term]["en"_ls]["name"_ls].toString();
            termData["url"_ls] = params["m.login.terms"_ls]["policies"_ls][term]["en"_ls]["url"_ls].toString();
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

    m_connection = new Connection(this);
    m_connection->resolveServer("@user:%1"_ls.arg(m_homeserver));
    connectSingleShot(m_connection.data(), &Connection::loginFlowsChanged, this, [this]() {
        if (m_testServerJob) {
            delete m_testServerJob;
        }
        m_testServerJob = m_connection->callApi<NeoChatRegisterJob>("user"_ls, none, "user"_ls, QString(), QString(), QString(), false);
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

QVector<QVariantMap> Registration::terms() const
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
                                       const Omittable<QJsonObject> &auth,
                                       const QString &username,
                                       const QString &password,
                                       const QString &deviceId,
                                       const QString &initialDeviceDisplayName,
                                       Omittable<bool> inhibitLogin)
    : BaseJob(HttpVerb::Post, "RegisterJob"_ls, QByteArrayLiteral("/_matrix/client/r0/register"), false)
{
    QJsonObject _data;
    if (auth) {
        addParam<>(_data, "auth"_ls, auth);
    }
    addParam<>(_data, "username"_ls, username);
    addParam<IfNotEmpty>(_data, "password"_ls, password);
    addParam<IfNotEmpty>(_data, "device_id"_ls, deviceId);
    addParam<IfNotEmpty>(_data, "initial_device_display_name"_ls, initialDeviceDisplayName);
    addParam<IfNotEmpty>(_data, "inhibit_login"_ls, inhibitLogin);
    addParam<IfNotEmpty>(_data, "kind"_ls, kind);
    addParam<IfNotEmpty>(_data, "refresh_token"_ls, false);
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
    connect(job, &BaseJob::finished, this, [=]() {
        m_sid = job->jsonData()["sid"_ls].toString();
    });
}
