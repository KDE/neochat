// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threepidaddhelper.h"

#include <Quotient/converters.h>
#include <Quotient/csapi/definitions/auth_data.h>
#include <Quotient/csapi/definitions/request_msisdn_validation.h>
#include <Quotient/jobs/basejob.h>

#include "jobs/neochatadd3pidjob.h"
#include "neochatconnection.h"

ThreePIdAddHelper::ThreePIdAddHelper(QObject *parent)
    : QObject(parent)
{
}

NeoChatConnection *ThreePIdAddHelper::connection() const
{
    return m_connection;
}

void ThreePIdAddHelper::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }
    m_connection = connection;
    Q_EMIT connectionChanged();
}

QString ThreePIdAddHelper::medium() const
{
    return m_medium;
}

void ThreePIdAddHelper::setMedium(const QString &medium)
{
    if (m_medium == medium) {
        return;
    }
    m_medium = medium;
    Q_EMIT mediumChanged();
}

QString ThreePIdAddHelper::newId() const
{
    return m_newId;
}

void ThreePIdAddHelper::setNewId(const QString &newId)
{
    if (newId == m_newId) {
        return;
    }
    m_newId = newId;
    Q_EMIT newIdChanged();

    m_newIdSecret.clear();
    m_newIdSid.clear();
    m_newIdStatus = Ready;
    Q_EMIT newIdStatusChanged();
}

QString ThreePIdAddHelper::newCountryCode() const
{
    return m_newCountryCode;
}

void ThreePIdAddHelper::setNewCountryCode(const QString &newCountryCode)
{
    if (newCountryCode == m_newCountryCode) {
        return;
    }
    m_newCountryCode = newCountryCode;
    Q_EMIT newCountryCodeChanged();

    m_newIdSecret.clear();
    m_newIdSid.clear();
    m_newIdStatus = Ready;
    Q_EMIT newIdStatusChanged();
}

void ThreePIdAddHelper::initiateNewIdAdd()
{
    if (m_newId.isEmpty()) {
        return;
    }
    if (m_medium == QLatin1String("email")) {
        emailTokenJob();
    } else {
        msisdnTokenJob();
    }
}

void ThreePIdAddHelper::emailTokenJob()
{
    m_newIdSecret = QString::fromLatin1(QUuid::createUuid().toString().toLatin1().toBase64());
    Quotient::EmailValidationData data;
    data.email = m_newId;
    data.clientSecret = m_newIdSecret;
    data.sendAttempt = 0;

    const auto job = m_connection->callApi<Quotient::RequestTokenTo3PIDEmailJob>(data);
    connect(job, &Quotient::BaseJob::finished, this, &ThreePIdAddHelper::tokenJobFinished);
}

void ThreePIdAddHelper::msisdnTokenJob()
{
    m_newIdSecret = QString::fromLatin1(QUuid::createUuid().toString().toLatin1().toBase64());
    Quotient::MsisdnValidationData data;
    data.country = m_newCountryCode;
    data.phoneNumber = m_newId;
    data.clientSecret = m_newIdSecret;
    data.sendAttempt = 0;

    const auto job = m_connection->callApi<Quotient::RequestTokenTo3PIDMSISDNJob>(data);
    connect(job, &Quotient::BaseJob::finished, this, &ThreePIdAddHelper::tokenJobFinished);
}

void ThreePIdAddHelper::tokenJobFinished(Quotient::BaseJob *job)
{
    if (job->status() == Quotient::BaseJob::Success) {
        m_newIdSid = job->jsonData()[QLatin1String("sid")].toString();
        m_newIdStatus = Verification;
        Q_EMIT newIdStatusChanged();
        return;
    }
    m_newIdStatus = Invalid;
    Q_EMIT newIdStatusChanged();
}

ThreePIdAddHelper::ThreePIdStatus ThreePIdAddHelper::newIdStatus() const
{
    return m_newIdStatus;
}

void ThreePIdAddHelper::finalizeNewIdAdd(const QString &password)
{
    const auto job = m_connection->callApi<NeochatAdd3PIdJob>(m_newIdSecret, m_newIdSid);
    connect(job, &Quotient::BaseJob::result, this, [this, job, password] {
        m_newIdStatus = Authentication;
        Q_EMIT newIdStatusChanged();

        if (static_cast<Quotient::BaseJob::StatusCode>(job->error()) == Quotient::BaseJob::Unauthorised) {
            QJsonObject replyData = job->jsonData();
            QJsonObject authData;
            authData[QLatin1String("session")] = replyData[QLatin1String("session")];
            authData[QLatin1String("password")] = password;
            authData[QLatin1String("type")] = QLatin1String("m.login.password");
            QJsonObject identifier = {{QLatin1String("type"), QLatin1String("m.id.user")}, {QLatin1String("user"), m_connection->userId()}};
            authData[QLatin1String("identifier")] = identifier;
            const auto innerJob = m_connection->callApi<NeochatAdd3PIdJob>(m_newIdSecret, m_newIdSid, authData);
            connect(innerJob, &Quotient::BaseJob::success, this, [this]() {
                m_connection->threePIdModel()->refreshModel();
                m_newIdSecret.clear();
                m_newIdSid.clear();
                m_newIdStatus = Success;
                Q_EMIT newIdStatusChanged();
            });
            connect(innerJob, &Quotient::BaseJob::failure, this, [innerJob, this]() {
                if (innerJob->jsonData()[QLatin1String("errcode")] == QLatin1String("M_FORBIDDEN")) {
                    m_newIdStatus = AuthFailure;
                    Q_EMIT newIdStatusChanged();
                } else if (innerJob->jsonData()[QLatin1String("errcode")] == QLatin1String("M_THREEPID_AUTH_FAILED")) {
                    m_newIdStatus = VerificationFailure;
                    Q_EMIT newIdStatusChanged();
                } else {
                    m_newIdStatus = Other;
                    Q_EMIT newIdStatusChanged();
                }
            });
        }
    });
}

void ThreePIdAddHelper::remove3PId(const QString &threePId, const QString &type)
{
    const auto job = m_connection->callApi<Quotient::Delete3pidFromAccountJob>(type, threePId);
    connect(job, &Quotient::BaseJob::success, this, [this]() {
        m_connection->threePIdModel()->refreshModel();
    });
}

void ThreePIdAddHelper::unbind3PId(const QString &threePId, const QString &type)
{
    const auto job = m_connection->callApi<Quotient::Unbind3pidFromAccountJob>(type, threePId);
    connect(job, &Quotient::BaseJob::success, this, [this]() {
        m_connection->threePIdModel()->refreshModel();
    });
}

void ThreePIdAddHelper::back()
{
    switch (m_newIdStatus) {
    case Verification:
    case Authentication:
    case AuthFailure:
    case VerificationFailure:
        m_newIdStatus = Ready;
        Q_EMIT newIdStatusChanged();
        return;
    default:
        return;
    }
}

#include "moc_threepidaddhelper.cpp"
