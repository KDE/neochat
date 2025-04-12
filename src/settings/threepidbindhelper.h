// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

#include <Quotient/jobs/basejob.h>

class NeoChatConnection;

/**
 * @class ThreePIdBindHelper
 *
 * This class is designed to help the process of bindind a 3PID to an identity server.
 * It will manage the various stages of verification and authentication.
 */
class ThreePIdBindHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The connection to bind a 3PID for.
     */
    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The type of 3PID being bound.
     *
     * email or msisdn.
     */
    Q_PROPERTY(QString medium READ medium WRITE setMedium NOTIFY mediumChanged)

    /**
     * @brief The 3PID to bind.
     *
     * Email or phone number depending on type.
     */
    Q_PROPERTY(QString newId READ newId WRITE setNewId NOTIFY newIdChanged)

    /**
     * @brief The country code if a phone number is being bound.
     */
    Q_PROPERTY(QString newCountryCode READ newCountryCode WRITE setNewCountryCode NOTIFY newCountryCodeChanged)

    /**
     * @brief The current status.
     *
     * @sa ThreePIdStatus
     */
    Q_PROPERTY(ThreePIdStatus bindStatus READ bindStatus NOTIFY bindStatusChanged)

    /**
     * @brief The current status as a string.
     *
     * @sa ThreePIdStatus
     */
    Q_PROPERTY(QString bindStatusString READ bindStatusString NOTIFY bindStatusChanged)

public:
    /**
     * @brief Defines the current status for binding a 3PID.
     */
    enum ThreePIdStatus {
        Ready, /**< The process is ready to start. I.e. there is no ongoing attempt to set a new 3PID. */
        Verification, /**< The request to verify the new 3PID has been sent. */
        Authentication, /**< The user needs to authenticate. */
        Success, /**< The 3PID has been successfully added. */
        Invalid, /**< The 3PID can't be used. */
        AuthFailure, /**< The authentication was wrong. */
        VerificationFailure, /**< The verification has not been completed. */
        Other, /**< An unknown problem occurred. */
    };
    Q_ENUM(ThreePIdStatus)

    explicit ThreePIdBindHelper(QObject *parent = nullptr);

    [[nodiscard]] NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

    [[nodiscard]] QString medium() const;
    void setMedium(const QString &medium);

    [[nodiscard]] QString newId() const;
    void setNewId(const QString &newEmail);

    [[nodiscard]] QString newCountryCode() const;
    void setNewCountryCode(const QString &newCountryCode);

    /**
     * @brief Start the process to bind the new 3PID.
     *
     * This will start the process of verifying the 3PID credentials that have been given.
     * Will fail if no identity server is configured.
     */
    Q_INVOKABLE void initiateNewIdBind();

    [[nodiscard]] ThreePIdStatus bindStatus() const;

    [[nodiscard]] QString bindStatusString() const;

    /**
     * @brief Finalize the process of binding the new 3PID.
     *
     * Will fail if the user hasn't completed the verification with the identity
     * server.
     */
    Q_INVOKABLE void finalizeNewIdBind();

    /**
     * @brief Unbind the given 3PID.
     */
    Q_INVOKABLE void unbind3PId(const QString &threePId, const QString &type);

    /**
     * @brief Cancel the process.
     */
    Q_INVOKABLE void cancel();

Q_SIGNALS:
    void connectionChanged();
    void mediumChanged();
    void newIdChanged();
    void newCountryCodeChanged();
    void newEmailSessionStartedChanged();
    void bindStatusChanged();

    /**
     * @brief A 3PID has been unbound.
     */
    void threePIdBound();

    /**
     * @brief A 3PID has been unbound.
     */
    void threePIdUnbound();

private:
    QPointer<NeoChatConnection> m_connection;
    QString m_medium = QString();

    ThreePIdStatus m_bindStatus = Ready;
    QString m_newId = QString();
    QString m_newCountryCode = QString();
    QString m_newIdSecret = QString();
    QString m_newIdSid = QString();
    QString m_identityServerToken = QString();

    QByteArray validationRequestData();

    void tokenRequestFinished(QNetworkReply *reply);

    static QJsonObject parseJson(const QByteArray &json);
};
