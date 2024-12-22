// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "identityserverhelper.h"

#include <QNetworkReply>

#include <KLocalizedString>

#include <Quotient/networkaccessmanager.h>

#include "neochatconnection.h"

using namespace Qt::StringLiterals;

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

    if (m_url == m_connection->identityServer().toString()) {
        m_status = Match;
        Q_EMIT statusChanged();
        return;
    }

    if (m_url.isEmpty()) {
        m_status = Valid;
        Q_EMIT statusChanged();
        return;
    }

    const auto requestUrl = QUrl(m_url + u"/_matrix/identity/v2"_s);
    if (!(requestUrl.scheme() == u"https"_s || requestUrl.scheme() == u"http"_s)) {
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
    if (m_url == m_connection->identityServer().toString()) {
        return;
    }

    m_connection->setAccountData(u"m.identity_server"_s, {{"base_url"_L1, m_url}});
    m_status = Ready;
    Q_EMIT statusChanged();
}

void IdentityServerHelper::clearIdentityServer()
{
    if (m_connection->identityServer().isEmpty()) {
        return;
    }
    m_connection->setAccountData(u"m.identity_server"_s, {{"base_url"_L1, QString()}});
    m_status = Ready;
    Q_EMIT statusChanged();
}

#include "moc_identityserverhelper.cpp"
