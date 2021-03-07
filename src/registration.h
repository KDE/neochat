/**
 * SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QVariantMap>

#include <connection.h>

#include <jobs/basejob.h>
#include <util.h>

class Registration : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString homeserver READ homeserver WRITE setHomeserver NOTIFY homeserverChanged)
    Q_PROPERTY(QString recaptchaSiteKey READ recaptchaSiteKey WRITE setRecaptchaSiteKey NOTIFY recaptchaSiteKeyChanged)
    Q_PROPERTY(QString recaptchaResponse READ recaptchaResponse WRITE setRecaptchaResponse NOTIFY recaptchaResponseChanged)
    Q_PROPERTY(bool homeserverAvailable READ homeserverAvailable NOTIFY homeserverAvailableChanged)
    Q_PROPERTY(bool testing READ testing NOTIFY testingChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(bool usernameAvailable READ usernameAvailable NOTIFY usernameAvailableChanged)
    Q_PROPERTY(bool testingUsername READ testingUsername NOTIFY testingUsernameChanged)
    Q_PROPERTY(QVector<QVariantMap> terms READ terms NOTIFY termsChanged)

public:

    static Registration &instance()
    {
        static Registration _instance;
        return _instance;
    }

    Q_INVOKABLE void registerAccount(const QString &homeserver, const QString &username, const QString &email, const QString &password);

    void setRecaptchaSiteKey(const QString &recaptchaSiteKey);
    QString recaptchaSiteKey() const;

    void setRecaptchaResponse(const QString &response);
    QString recaptchaResponse() const;

    void setTermsName(const QString &termsname);
    QString termsName() const;

    void setTermsUrl(const QString &termsUrl);
    QString termsUrl() const;

    void setHomeserver(const QString &url);
    QString homeserver() const;

    bool homeserverAvailable() const;
    void setHomeserverAvailable(bool available);

    bool testing() const;
    void setTesting(bool testing);

    QString username() const;
    void setUsername(const QString &username);

    void setUsernameAvailable(bool available);
    bool usernameAvailable() const;

    void setTestingUsername(bool testing);
    bool testingUsername() const;

    QVector<QVariantMap> terms() const;

Q_SIGNALS:
    void recaptchaSiteKeyChanged();
    void recaptchaResponseChanged();
    void termsNameChanged();
    void termsUrlChanged();
    void homeserverChanged();
    void homeserverAvailableChanged();
    void testingChanged();
    void usernameChanged();
    void usernameAvailableChanged();
    void testingUsernameChanged();
    void flowsChanged();
    void termsChanged();

private:
    QString m_recaptchaSiteKey = "6LcgI54UAAAAABGdGmruw6DdOocFpYVdjYBRe4zb";
    QString m_recaptchaResponse;
    QString m_termsName;
    QString m_termsUrl;
    QString m_homeserver;
    QString m_username;
    QVector<QVariantMap> m_terms;

    bool m_homeserverAvailable = false;
    bool m_testing = false;
    bool m_usernameAvailable = false;
    bool m_testingUsername = false;

    QVector<QVector<QString>> m_flows;

    Quotient::Connection *m_connection = nullptr;

    void testHomeserver();
    void testUsername();
    void loadFlows();

    Registration();
};

class NeochatRegisterJob : public Quotient::BaseJob
{
public:
    explicit NeochatRegisterJob(const QString& kind = QStringLiteral("user"),
                         const Quotient::Omittable<QJsonObject>& auth = Quotient::none,
                         const QString& username = {},
                         const QString& password = {},
                         const QString& deviceId = {},
                         const QString& initialDeviceDisplayName = {},
                         Quotient::Omittable<bool> inhibitLogin = Quotient::none);

    QString userId() const { return loadFromJson<QString>(QStringLiteral("user_id")); }

    QString accessToken() const
    {
        return loadFromJson<QString>(QStringLiteral("access_token"));
    }

    QString homeServer() const
    {
        return loadFromJson<QString>(QStringLiteral("home_server"));
    }

    QString deviceId() const { return loadFromJson<QString>(QStringLiteral("device_id")); }
};
    
