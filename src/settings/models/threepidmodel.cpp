// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threepidmodel.h"

#include <QCryptographicHash>
#include <QNetworkReply>

#include <Quotient/csapi/openid.h>
#include <Quotient/networkaccessmanager.h>

#include "neochatconnection.h"

using namespace Qt::StringLiterals;

ThreePIdModel::ThreePIdModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

NeoChatConnection *ThreePIdModel::connection() const
{
    return m_connection;
}

void ThreePIdModel::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }

    if (m_connection != nullptr) {
        m_connection->disconnect(this);
    }

    m_connection = connection;
    if (m_connection) {
        connect(m_connection, &NeoChatConnection::stateChanged, this, [this]() {
            refreshModel();
        });
        refreshModel();
    }

    Q_EMIT connectionChanged();
}

QVariant ThreePIdModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= rowCount()) {
        qDebug() << "ThreePIdModel, something's wrong: index.row() >= m_threePIds.count()";
        return {};
    }

    if (role == AddressRole) {
        return m_threePIds.at(index.row()).address;
    }
    if (role == MediumRole) {
        return m_threePIds.at(index.row()).medium;
    }
    if (role == IsBoundRole) {
        return m_bindings.contains(m_threePIds.at(index.row()).address);
    }

    return {};
}

int ThreePIdModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_threePIds.count();
}

QHash<int, QByteArray> ThreePIdModel::roleNames() const
{
    return {
        {AddressRole, QByteArrayLiteral("address")},
        {MediumRole, QByteArrayLiteral("medium")},
        {IsBoundRole, QByteArrayLiteral("isBound")},
    };
}

void ThreePIdModel::refreshModel()
{
    if (m_connection != nullptr && m_connection->isLoggedIn()) {
        if (m_job.isRunning()) {
            m_job.cancel();
        }
        m_job = m_connection->callApi<Quotient::GetAccount3PIDsJob>();
        connect(m_job, &Quotient::BaseJob::success, this, [this]() {
            beginResetModel();
            m_threePIds = m_job->threepids();
            endResetModel();

            refreshBindStatus();
        });
    }
}

void ThreePIdModel::refreshBindStatus()
{
    if (m_connection == nullptr || !m_connection->hasIdentityServer()) {
        return;
    }

    const auto openIdJob = m_connection->callApi<Quotient::RequestOpenIdTokenJob>(m_connection->userId());
    connect(openIdJob, &Quotient::BaseJob::success, this, [this, openIdJob]() {
        const auto requestUrl = QUrl(m_connection->identityServer().toString() + u"/_matrix/identity/v2/account/register"_s);
        if (!(requestUrl.scheme() == u"https"_s || requestUrl.scheme() == u"http"_s)) {
            return;
        }

        QNetworkRequest request(requestUrl);
        auto newRequest = Quotient::NetworkAccessManager::instance()->post(request, QJsonDocument(openIdJob->jsonData()).toJson());
        connect(newRequest, &QNetworkReply::finished, this, [this, newRequest]() {
            QJsonObject replyJson = QJsonDocument::fromJson(newRequest->readAll()).object();
            const auto identityServerToken = replyJson["token"_L1].toString();

            const auto requestUrl = QUrl(m_connection->identityServer().toString() + u"/_matrix/identity/v2/hash_details"_s);
            if (!(requestUrl.scheme() == u"https"_s || requestUrl.scheme() == u"http"_s)) {
                return;
            }

            QNetworkRequest hashRequest(requestUrl);
            hashRequest.setRawHeader("Authorization", "Bearer " + identityServerToken.toLatin1());

            auto hashReply = Quotient::NetworkAccessManager::instance()->get(hashRequest);
            connect(hashReply, &QNetworkReply::finished, this, [this, identityServerToken, hashReply]() {
                QJsonObject replyJson = QJsonDocument::fromJson(hashReply->readAll()).object();
                const auto lookupPepper = replyJson["lookup_pepper"_L1].toString();

                const auto requestUrl = QUrl(m_connection->identityServer().toString() + u"/_matrix/identity/v2/lookup"_s);
                if (!(requestUrl.scheme() == u"https"_s || requestUrl.scheme() == u"http"_s)) {
                    return;
                }

                QNetworkRequest lookupRequest(requestUrl);
                lookupRequest.setRawHeader("Authorization", "Bearer " + identityServerToken.toLatin1());

                QJsonObject requestData = {
                    {"algorithm"_L1, "none"_L1},
                    {"pepper"_L1, lookupPepper},
                };
                QJsonArray idLookups;
                for (const auto &id : m_threePIds) {
                    idLookups += u"%1 %2"_s.arg(id.address, id.medium);
                }
                requestData["addresses"_L1] = idLookups;

                auto lookupReply = Quotient::NetworkAccessManager::instance()->post(lookupRequest, QJsonDocument(requestData).toJson(QJsonDocument::Compact));
                connect(lookupReply, &QNetworkReply::finished, this, [this, lookupReply]() {
                    beginResetModel();
                    m_bindings.clear();

                    QJsonObject mappings = QJsonDocument::fromJson(lookupReply->readAll()).object()["mappings"_L1].toObject();
                    for (const auto &id : mappings.keys()) {
                        if (mappings[id] == m_connection->userId()) {
                            m_bindings += id.section(u' ', 0, 0);
                        }
                    }

                    endResetModel();
                });
            });
        });
    });
}

#include "moc_threepidmodel.cpp"
