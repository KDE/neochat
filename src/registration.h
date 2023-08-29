// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantMap>
#include <QVector>

#include <Quotient/csapi/registration.h>

#include <Quotient/connection.h>

#include <Quotient/jobs/basejob.h>
#include <Quotient/util.h>

namespace Quotient
{
class Connection;
class CheckUsernameAvailabilityJob;
}

class NeoChatRegisterJob : public Quotient::BaseJob
{
public:
    explicit NeoChatRegisterJob(const QString &kind = QStringLiteral("user"),
                                const Quotient::Omittable<QJsonObject> &auth = Quotient::none,
                                const QString &username = {},
                                const QString &password = {},
                                const QString &deviceId = {},
                                const QString &initialDeviceDisplayName = {},
                                Quotient::Omittable<bool> inhibitLogin = Quotient::none);

    QString userId() const
    {
        return loadFromJson<QString>(QStringLiteral("user_id"));
    }

    QString accessToken() const
    {
        return loadFromJson<QString>(QStringLiteral("access_token"));
    }

    QString homeServer() const
    {
        return loadFromJson<QString>(QStringLiteral("home_server"));
    }

    QString deviceId() const
    {
        return loadFromJson<QString>(QStringLiteral("device_id"));
    }
};

class Registration : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString homeserver READ homeserver WRITE setHomeserver NOTIFY homeserverChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString recaptchaSiteKey READ recaptchaSiteKey WRITE setRecaptchaSiteKey NOTIFY recaptchaSiteKeyChanged)
    Q_PROPERTY(QString recaptchaResponse READ recaptchaResponse WRITE setRecaptchaResponse NOTIFY recaptchaResponseChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString nextStep READ nextStep WRITE setNextStep NOTIFY nextStepChanged)
    Q_PROPERTY(QVector<QVariantMap> terms READ terms NOTIFY termsChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)

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
    };
    Q_ENUM(Status);
    static Registration &instance()
    {
        static Registration _instance;
        return _instance;
    }

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

    QVector<QVariantMap> terms() const;

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

private:
    QString m_recaptchaSiteKey;
    QString m_recaptchaResponse;
    QString m_homeserver;
    QString m_username;
    QString m_password;
    QVector<QVariantMap> m_terms;
    QString m_email;
    Status m_status = NoServer;
    QString m_nextStep;
    QString m_session;
    QString m_sid;
    QString m_emailSecret;

    QPointer<Quotient::CheckUsernameAvailabilityJob> m_usernameJob;
    QPointer<NeoChatRegisterJob> m_testServerJob;
    QVector<QVector<QString>> m_flows;
    QPointer<Quotient::Connection> m_connection;

    void testHomeserver();
    void testUsername();
    void setStatus(Status status);

    Registration();
};
