// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

#include <Quotient/jobs/basejob.h>

class NeoChatConnection;

/**
 * @class ThreePIdAddHelper
 *
 * This class is designed to help the process of adding a new 3PID to the account.
 * It will manage the various stages of verification and authentication.
 */
class ThreePIdAddHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The connection to add a 3PID to.
     */
    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The type of 3PID being added.
     *
     * email or msisdn.
     */
    Q_PROPERTY(QString medium READ medium WRITE setMedium NOTIFY mediumChanged)

    /**
     * @brief The 3PID to add.
     *
     * Email or phone number depending on type.
     */
    Q_PROPERTY(QString newId READ newId WRITE setNewId NOTIFY newIdChanged)

    /**
     * @brief The country code if a phone number is being added.
     */
    Q_PROPERTY(QString newCountryCode READ newCountryCode WRITE setNewCountryCode NOTIFY newCountryCodeChanged)

    /**
     * @brief The current status.
     *
     * @sa ThreePIdStatus
     */
    Q_PROPERTY(ThreePIdStatus newIdStatus READ newIdStatus NOTIFY newIdStatusChanged)

public:
    /**
     * @brief Defines the current status for adding a 3PID.
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

    explicit ThreePIdAddHelper(QObject *parent = nullptr);

    [[nodiscard]] NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

    [[nodiscard]] QString medium() const;
    void setMedium(const QString &medium);

    [[nodiscard]] QString newId() const;
    void setNewId(const QString &newEmail);

    [[nodiscard]] QString newCountryCode() const;
    void setNewCountryCode(const QString &newCountryCode);

    /**
     * @brief Start the process to add the new 3PID.
     *
     * This will start the process of verifying the 3PID credentials that have been given.
     */
    Q_INVOKABLE void initiateNewIdAdd();

    [[nodiscard]] ThreePIdStatus newIdStatus() const;

    /**
     * @brief Finalize the process of adding the new 3PID.
     *
     * @param password the user's password to authenticate the addition.
     */
    Q_INVOKABLE void finalizeNewIdAdd(const QString &password);

    /**
     * @brief Remove the given 3PID.
     */
    Q_INVOKABLE void remove3PId(const QString &threePId, const QString &type);

    /**
     * @brief Remove the given 3PID.
     */
    Q_INVOKABLE void unbind3PId(const QString &threePId, const QString &type);

    /**
     * @brief Go back a step in the process.
     */
    Q_INVOKABLE void back();

Q_SIGNALS:
    void connectionChanged();
    void mediumChanged();
    void newIdChanged();
    void newCountryCodeChanged();
    void newEmailSessionStartedChanged();
    void newIdStatusChanged();

    /**
     * @brief A 3PID has been added.
     */
    void threePIdAdded();

    /**
     * @brief A 3PID has been removed.
     */
    void threePIdRemoved();

    /**
     * @brief A 3PID has been unbound.
     */
    void threePIdUnbound();

private:
    QPointer<NeoChatConnection> m_connection;
    QString m_medium = QString();

    ThreePIdStatus m_newIdStatus = Ready;
    QString m_newId = QString();
    QString m_newCountryCode = QString();
    QString m_newIdSecret = QString();
    QString m_newIdSid = QString();

    void emailTokenJob();
    void msisdnTokenJob();

    void tokenJobFinished(Quotient::BaseJob *job);
};
