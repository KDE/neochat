// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QVariantMap>

#include <Quotient/csapi/registration.h>

#include <Quotient/jobs/basejob.h>
#include <Quotient/util.h>

#include "accountmanager.h"

using namespace Qt::StringLiterals;

namespace Quotient
{
class CheckUsernameAvailabilityJob;
}

class NeoChatConnection;

class NeoChatRegisterJob : public Quotient::BaseJob
{
public:
    explicit NeoChatRegisterJob(const QString &kind = u"user"_s,
                                const std::optional<QJsonObject> &auth = {},
                                const QString &username = {},
                                const QString &password = {},
                                const QString &deviceId = {},
                                const QString &initialDeviceDisplayName = {},
                                std::optional<bool> inhibitLogin = {});

    QString userId() const
    {
        return loadFromJson<QString>(u"user_id"_s);
    }

    QString accessToken() const
    {
        return loadFromJson<QString>(u"access_token"_s);
    }

    QString homeServer() const
    {
        return loadFromJson<QString>(u"home_server"_s);
    }

    QString deviceId() const
    {
        return loadFromJson<QString>(u"device_id"_s);
    }
};

class Registration : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString homeserver READ homeserver WRITE setHomeserver NOTIFY homeserverChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString recaptchaSiteKey READ recaptchaSiteKey WRITE setRecaptchaSiteKey NOTIFY recaptchaSiteKeyChanged)
    Q_PROPERTY(QString recaptchaResponse READ recaptchaResponse WRITE setRecaptchaResponse NOTIFY recaptchaResponseChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString nextStep READ nextStep WRITE setNextStep NOTIFY nextStepChanged)
    Q_PROPERTY(QList<QVariantMap> terms READ terms NOTIFY termsChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)
    Q_PROPERTY(QUrl oidcUrl MEMBER m_oidcUrl NOTIFY oidcUrlChanged)

public:
    enum Status {
        NoServer,
        TestingHomeserver,
        InvalidServer,
        ServerNoRegistration,
        NoUsername,
        TestingUsername,
        UsernameTaken,
        Ready,
        Working,
        Oidc,
    };
    Q_ENUM(Status);
    static Registration &instance()
    {
        static Registration _instance;
        return _instance;
    }
    static Registration *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    void setAccountManager(AccountManager *manager);

    Q_INVOKABLE void registerAccount();
    Q_INVOKABLE void registerEmail();

    void setRecaptchaSiteKey(const QString &recaptchaSiteKey);
    QString recaptchaSiteKey() const;

    void setRecaptchaResponse(const QString &response);
    QString recaptchaResponse() const;

    void setHomeserver(const QString &url);
    QString homeserver() const;

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    [[nodiscard]] QString email() const;
    void setEmail(const QString &email);

    QString nextStep() const;
    void setNextStep(const QString &nextStep);

    QList<QVariantMap> terms() const;

    Status status() const;
    QString statusString() const;

Q_SIGNALS:
    void recaptchaSiteKeyChanged();
    void recaptchaResponseChanged();
    void homeserverChanged();
    void homeserverAvailableChanged();
    void testingChanged();
    void usernameChanged();
    void usernameAvailableChanged();
    void testingUsernameChanged();
    void flowsChanged();
    void termsChanged();
    void passwordChanged();
    void emailChanged();
    void nextStepChanged();
    void statusChanged();
    void loaded();
    void oidcUrlChanged();
    void connected(NeoChatConnection *connection);

private:
    QPointer<AccountManager> m_accountManager;

    QString m_recaptchaSiteKey;
    QString m_recaptchaResponse;
    QString m_homeserver;
    QString m_username;
    QString m_password;
    QList<QVariantMap> m_terms;
    QString m_email;
    Status m_status = NoServer;
    QString m_nextStep;
    QString m_session;
    QString m_sid;
    QString m_emailSecret;
    QUrl m_oidcUrl;

    QPointer<Quotient::CheckUsernameAvailabilityJob> m_usernameJob;
    QPointer<NeoChatRegisterJob> m_testServerJob;
    QList<QList<QString>> m_flows;
    QPointer<NeoChatConnection> m_connection;

    void testHomeserver();
    void testUsername();
    void setStatus(Status status);

    Registration();
};
