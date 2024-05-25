// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "identityserverhelper.h"

#include <QNetworkReply>

#include <KLocalizedString>

#include <Quotient/networkaccessmanager.h>

#include "neochatconnection.h"

IdentityServerHelper::IdentityServerHelper(QObject *parent)
    : QObject(parent)
{
}

NeoChatConnection *IdentityServerHelper::connection() const
{
    return m_connection;
}

void IdentityServerHelper::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }

    if (m_connection != nullptr) {
        m_connection->disconnect(this);
    }

    m_connection = connection;
    Q_EMIT connectionChanged();
    Q_EMIT currentServerChanged();

    connect(m_connection, &NeoChatConnection::accountDataChanged, this, [this](const QString &type) {
        if (type == QLatin1String("m.identity_server")) {
            Q_EMIT currentServerChanged();
        }
    });
}

QString IdentityServerHelper::currentServer() const
{
    if (m_connection == nullptr) {
        return {};
    }

    if (!m_connection->hasAccountData(QLatin1String("m.identity_server"))) {
        return i18nc("@info", "No identity server configured");
    }

    const auto url = m_connection->accountData(QLatin1String("m.identity_server"))->contentPart<QUrl>(QLatin1String("base_url"));
    if (!url.isEmpty()) {
        return url.toString();
    }
    return i18nc("@info", "No identity server configured");
}

bool IdentityServerHelper::hasCurrentServer() const
{
    if (m_connection == nullptr && !m_connection->hasAccountData(QLatin1String("m.identity_server"))) {
        return false;
    }

    const auto url = m_connection->accountData(QLatin1String("m.identity_server"))->contentPart<QUrl>(QLatin1String("base_url"));
    if (!url.isEmpty()) {
        return true;
    }
    return false;
}

QString IdentityServerHelper::url() const
{
    return m_url;
}

void IdentityServerHelper::setUrl(const QString &url)
{
    if (url == m_url) {
        return;
    }
    m_url = url;
    Q_EMIT urlChanged();

    checkUrl();
}

IdentityServerHelper::IdServerStatus IdentityServerHelper::status() const
{
    return m_status;
}

void IdentityServerHelper::checkUrl()
{
    if (m_idServerCheckRequest != nullptr) {
        m_idServerCheckRequest->abort();
        m_idServerCheckRequest.clear();
    }

    if (m_url == currentServer()) {
        m_status = Match;
        Q_EMIT statusChanged();
        return;
    }

    if (m_url.isEmpty()) {
        m_status = Valid;
        Q_EMIT statusChanged();
        return;
    }

    const auto requestUrl = QUrl(m_url + QStringLiteral("/_matrix/identity/v2"));
    if (!(requestUrl.scheme() == QStringLiteral("https") || requestUrl.scheme() == QStringLiteral("http"))) {
        m_status = Invalid;
        Q_EMIT statusChanged();
        return;
    }

    QNetworkRequest request(requestUrl);
    m_idServerCheckRequest = Quotient::NetworkAccessManager::instance()->get(request);
    connect(m_idServerCheckRequest, &QNetworkReply::finished, this, [this]() {
        if (m_idServerCheckRequest->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
            m_status = Valid;
            Q_EMIT statusChanged();
        } else {
            m_status = Invalid;
            Q_EMIT statusChanged();
        }
    });
}

void IdentityServerHelper::setIdentityServer()
{
    if (m_url == currentServer()) {
        return;
    }

    m_connection->setAccountData(QLatin1String("m.identity_server"), {{QLatin1String("base_url"), m_url}});
    m_status = Ready;
    Q_EMIT statusChanged();
}

void IdentityServerHelper::clearIdentityServer()
{
    if (currentServer().isEmpty()) {
        return;
    }
    m_connection->setAccountData(QLatin1String("m.identity_server"), {{QLatin1String("base_url"), QString()}});
    m_status = Ready;
    Q_EMIT statusChanged();
}

#include "moc_identityserverhelper.cpp"
