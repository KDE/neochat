// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threepidbindhelper.h"

#include <QNetworkReply>

#include <Quotient/converters.h>
#include <Quotient/csapi/definitions/auth_data.h>
#include <Quotient/csapi/definitions/request_msisdn_validation.h>
#include <Quotient/csapi/openid.h>
#include <Quotient/networkaccessmanager.h>

#include <KLocalizedString>

#include "neochatconnection.h"

ThreePIdBindHelper::ThreePIdBindHelper(QObject *parent)
    : QObject(parent)
{
}

NeoChatConnection *ThreePIdBindHelper::connection() const
{
    return m_connection;
}

void ThreePIdBindHelper::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }
    m_connection = connection;
    Q_EMIT connectionChanged();
}

QString ThreePIdBindHelper::medium() const
{
    return m_medium;
}

void ThreePIdBindHelper::setMedium(const QString &medium)
{
    if (m_medium == medium) {
        return;
    }
    m_medium = medium;
    Q_EMIT mediumChanged();
}

QString ThreePIdBindHelper::newId() const
{
    return m_newId;
}

void ThreePIdBindHelper::setNewId(const QString &newId)
{
    if (newId == m_newId) {
        return;
    }
    m_newId = newId;
    Q_EMIT newIdChanged();

    m_newIdSecret.clear();
    m_newIdSid.clear();
    m_identityServerToken.clear();
    m_bindStatus = Ready;
    Q_EMIT bindStatusChanged();
}

QString ThreePIdBindHelper::newCountryCode() const
{
    return m_newCountryCode;
}

void ThreePIdBindHelper::setNewCountryCode(const QString &newCountryCode)
{
    if (newCountryCode == m_newCountryCode) {
        return;
    }
    m_newCountryCode = newCountryCode;
    Q_EMIT newCountryCodeChanged();

    m_newIdSecret.clear();
    m_newIdSid.clear();
    m_identityServerToken.clear();
    m_bindStatus = Ready;
    Q_EMIT bindStatusChanged();
}

void ThreePIdBindHelper::initiateNewIdBind()
{
    if (m_newId.isEmpty() || m_connection == nullptr || !m_connection->hasIdentityServer()) {
        return;
    }

    const auto openIdJob = m_connection->callApi<Quotient::RequestOpenIdTokenJob>(m_connection->userId());
    connect(openIdJob, &Quotient::BaseJob::success, this, [this, openIdJob]() {
        const auto requestUrl = QUrl(m_connection->identityServer().toString() + QStringLiteral("/_matrix/identity/v2/account/register"));
        if (!(requestUrl.scheme() == QStringLiteral("https") || requestUrl.scheme() == QStringLiteral("http"))) {
            m_bindStatus = AuthFailure;
            Q_EMIT bindStatusChanged();
            return;
        }

        QNetworkRequest request(requestUrl);
        auto newRequest = Quotient::NetworkAccessManager::instance()->post(request, QJsonDocument(openIdJob->jsonData()).toJson());
        connect(newRequest, &QNetworkReply::finished, this, [this, newRequest]() {
            QJsonObject replyJson = parseJson(newRequest->readAll());
            m_identityServerToken = replyJson[QLatin1String("token")].toString();

            const auto requestUrl = QUrl(m_connection->identityServer().toString() + QStringLiteral("/_matrix/identity/v2/validate/email/requestToken"));
            if (!(requestUrl.scheme() == QStringLiteral("https") || requestUrl.scheme() == QStringLiteral("http"))) {
                m_bindStatus = AuthFailure;
                Q_EMIT bindStatusChanged();
                return;
            }

            QNetworkRequest validationRequest(requestUrl);
            validationRequest.setRawHeader("Authorization", "Bearer " + m_identityServerToken.toLatin1());

            auto tokenRequest = Quotient::NetworkAccessManager::instance()->post(validationRequest, validationRequestData());
            connect(tokenRequest, &QNetworkReply::finished, this, [this, tokenRequest]() {
                tokenRequestFinished(tokenRequest);
            });
        });
    });
}

QByteArray ThreePIdBindHelper::validationRequestData()
{
    m_newIdSecret = QString::fromLatin1(QUuid::createUuid().toString().toLatin1().toBase64());
    QJsonObject requestData = {
        {QLatin1String("client_secret"), m_newIdSecret},
        {QLatin1String("send_attempt"), 0},
    };

    if (m_medium == QLatin1String("email")) {
        requestData[QLatin1String("email")] = m_newId;
    } else {
        requestData[QLatin1String("phone_number")] = m_newId;
        requestData[QLatin1String("country")] = m_newCountryCode;
    }

    return QJsonDocument(requestData).toJson();
}

void ThreePIdBindHelper::tokenRequestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    QJsonObject replyJson = parseJson(reply->readAll());
    m_newIdSid = replyJson[QLatin1String("sid")].toString();

    if (m_newIdSid.isEmpty()) {
        m_bindStatus = Invalid;
        Q_EMIT bindStatusChanged();
    } else {
        m_bindStatus = Verification;
        Q_EMIT bindStatusChanged();
    }
}

ThreePIdBindHelper::ThreePIdStatus ThreePIdBindHelper::bindStatus() const
{
    return m_bindStatus;
}

QString ThreePIdBindHelper::bindStatusString() const
{
    switch (m_bindStatus) {
    case Verification:
        return i18n("%1. Please follow the instructions there and then click the button above",
                    m_medium == QStringLiteral("email") ? i18n("We've sent you an email") : i18n("We've sent you a text message"));
    case Invalid:
        return m_medium == QStringLiteral("email") ? i18n("The entered email is not valid") : i18n("The entered phone number is not valid");
    case VerificationFailure:
        return m_medium == QStringLiteral("email")
            ? i18n("The email has not been verified. Please go to the email and follow the instructions there and then click the button above")
            : i18n("The phone number has not been verified. Please go to the text message and follow the instructions there and then click the button above");
    default:
        return {};
    }
}

void ThreePIdBindHelper::finalizeNewIdBind()
{
    const auto job = m_connection->callApi<Quotient::Bind3PIDJob>(m_newIdSecret, m_connection->identityServer().host(), m_identityServerToken, m_newIdSid);
    connect(job, &Quotient::BaseJob::success, this, [this] {
        m_bindStatus = Success;
        Q_EMIT bindStatusChanged();
        m_connection->threePIdModel()->refreshModel();
    });
    connect(job, &Quotient::BaseJob::failure, this, [this, job]() {
        if (job->jsonData()[QLatin1String("errcode")] == QLatin1String("M_SESSION_NOT_VALIDATED")) {
            m_bindStatus = VerificationFailure;
            Q_EMIT bindStatusChanged();
        } else {
            m_bindStatus = Other;
            Q_EMIT bindStatusChanged();
        }
    });
}

void ThreePIdBindHelper::unbind3PId(const QString &threePId, const QString &type)
{
    const auto job = m_connection->callApi<Quotient::Unbind3pidFromAccountJob>(type, threePId);
    connect(job, &Quotient::BaseJob::success, this, [this]() {
        cancel();
        m_connection->threePIdModel()->refreshModel();
    });
}

void ThreePIdBindHelper::cancel()
{
    m_newIdSecret.clear();
    m_newIdSid.clear();
    m_identityServerToken.clear();
    m_bindStatus = Ready;
    Q_EMIT bindStatusChanged();
}

QJsonObject ThreePIdBindHelper::parseJson(const QByteArray &json)
{
    const auto document = QJsonDocument::fromJson(json);
    return document.object();
}

#include "moc_threepidbindhelper.cpp"
