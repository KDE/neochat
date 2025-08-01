// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "accountmanager.h"

#include <QCoreApplication>
#include <QTimer>

#include <KLocalizedString>

#include <Quotient/settings.h>

#include "neochatroom.h"

using namespace Qt::StringLiterals;

AccountManager::AccountManager(bool testMode, QObject *parent)
    : QObject(parent)
    , m_accountRegistry(new Quotient::AccountRegistry(this))
{
    if (!testMode) {
        QTimer::singleShot(0, this, [this] {
            loadAccountsFromCache();
        });
    } else {
        auto c = new NeoChatConnection(QUrl(u"https://localhost:1234"_s), this);
        c->assumeIdentity(u"@user:localhost:1234"_s, u"device_1234"_s, u"token_1234"_s);
        m_accountRegistry->add(c);
        c->syncLoop();
    }
}

Quotient::AccountRegistry *AccountManager::accounts()
{
    return m_accountRegistry;
}

void AccountManager::loadAccountsFromCache()
{
    const auto accounts = Quotient::SettingsGroup("Accounts"_L1).childGroups();
    for (const auto &accountId : accounts) {
        Quotient::AccountSettings account{accountId};
        m_accountsLoading += accountId;
        Q_EMIT accountsLoadingChanged();
        if (!account.homeserver().isEmpty()) {
            auto accessTokenLoadingJob = loadAccessTokenFromKeyChain(account.userId());
            connect(accessTokenLoadingJob, &QKeychain::Job::finished, this, [accountId, this, accessTokenLoadingJob](QKeychain::Job *) {
                Quotient::AccountSettings account{accountId};
                QString accessToken;
                if (accessTokenLoadingJob->error() == QKeychain::Error::NoError) {
                    accessToken = QString::fromLatin1(accessTokenLoadingJob->binaryData());
                } else {
                    return;
                }

                auto connection = new NeoChatConnection(account.homeserver());
                m_connectionsLoading[accountId] = connection;
                connect(connection, &NeoChatConnection::connected, this, [this, connection, accountId] {
                    connection->loadState();
                    if (connection->allRooms().size() == 0 || connection->allRooms()[0]->currentState().get<Quotient::RoomCreateEvent>()) {
                        addConnection(connection);
                        m_accountsLoading.removeAll(connection->userId());
                        m_connectionsLoading.remove(accountId);
                        Q_EMIT accountsLoadingChanged();
                    } else {
                        connect(
                            connection->allRooms()[0],
                            &NeoChatRoom::baseStateLoaded,
                            this,
                            [this, connection, accountId]() {
                                addConnection(connection);
                                m_accountsLoading.removeAll(connection->userId());
                                m_connectionsLoading.remove(accountId);
                                Q_EMIT accountsLoadingChanged();
                            },
                            Qt::SingleShotConnection);
                    }
                });
                connection->assumeIdentity(account.userId(), account.deviceId(), accessToken);
            });
        }
    }
}

QStringList AccountManager::accountsLoading() const
{
    return m_accountsLoading;
}

void AccountManager::saveAccessTokenToKeyChain(NeoChatConnection *connection)
{
    if (!connection) {
        return;
    }
    const auto userId = connection->userId();

    qDebug() << "Save the access token to the keychain for " << userId;
    auto job = new QKeychain::WritePasswordJob(qAppName());
    job->setAutoDelete(true);
    job->setKey(userId);
    job->setBinaryData(connection->accessToken());
    connect(job, &QKeychain::WritePasswordJob::finished, this, [job]() {
        if (job->error()) {
            qWarning() << "Could not save access token to the keychain: " << qPrintable(job->errorString());
        }
    });
    job->start();
}

QKeychain::ReadPasswordJob *AccountManager::loadAccessTokenFromKeyChain(const QString &userId)
{
    qDebug() << "Reading access token from the keychain for" << userId;
    auto job = new QKeychain::ReadPasswordJob(qAppName(), this);
    job->setKey(userId);

    // Handling of errors
    connect(job, &QKeychain::Job::finished, this, [this, job]() {
        if (job->error() == QKeychain::Error::NoError) {
            return;
        }

        switch (job->error()) {
        case QKeychain::EntryNotFound:
            Q_EMIT errorOccured(i18n("Access token wasn't found: Maybe it was deleted?"));
            break;
        case QKeychain::AccessDeniedByUser:
        case QKeychain::AccessDenied:
            Q_EMIT errorOccured(i18n("Access to keychain was denied: Please allow NeoChat to read the access token"));
            break;
        case QKeychain::NoBackendAvailable:
            Q_EMIT errorOccured(i18n("No keychain available: Please install a keychain, e.g. KWallet or GNOME keyring on Linux"));
            break;
        case QKeychain::OtherError:
            Q_EMIT errorOccured(i18n("Unable to read access token: %1", job->errorString()));
            break;
        default:
            break;
        }
    });
    job->start();

    return job;
}

NeoChatConnection *AccountManager::activeConnection() const
{
    return m_activeConnection;
}

void AccountManager::setActiveConnection(NeoChatConnection *connection)
{
    if (connection == m_activeConnection) {
        return;
    }

    const auto oldConnection = m_activeConnection;
    m_activeConnection = connection;
    Q_EMIT activeConnectionChanged(oldConnection, m_activeConnection);
}

void AccountManager::addConnection(NeoChatConnection *connection)
{
    Q_ASSERT_X(connection, __FUNCTION__, "Attempt to add a null connection");

    saveAccessTokenToKeyChain(connection);
    m_accountRegistry->add(connection);

    connection->setLazyLoading(true);

    connect(connection, &NeoChatConnection::syncDone, this, [connection] {
        connection->sync(30000);
        connection->saveState();
    });
    connect(connection, &NeoChatConnection::loggedOut, this, [this, connection] {
        // Only set the connection if the account being logged out is currently active
        if (m_accountRegistry->accounts().count() == 1 && connection == activeConnection()) {
            setActiveConnection(dynamic_cast<NeoChatConnection *>(m_accountRegistry->accounts()[0]));
        } else {
            setActiveConnection(nullptr);
        }

        dropConnection(connection);
    });

    connection->sync();

    Q_EMIT connectionAdded(connection);
}

void AccountManager::dropConnection(const QString &userId)
{
    if (userId.isEmpty()) {
        return;
    }

    // There are 3 possible states:
    //  - in m_accountsLoading trying to loadAccessTokenFromKeyChain()
    //  - in m_connectionsLoading
    //  - in the AccountRegistry
    // Check all locations.

    if (dropAccountLoading(userId)) {
        return;
    }
    if (dropConnectionLoading(m_connectionsLoading.value(userId, nullptr))) {
        return;
    }
    const auto connection = dynamic_cast<NeoChatConnection *>(m_accountRegistry->get(userId));
    if (connection) {
        dropRegistry(connection);
    }
}

void AccountManager::dropConnection(NeoChatConnection *connection)
{
    if (!connection) {
        return;
    }

    // There are 3 possible states:
    //  - in m_accountsLoading trying to loadAccessTokenFromKeyChain()
    //  - in m_connectionsLoading
    //  - in the AccountRegistry
    // Check all locations.

    if (dropAccountLoading(connection->userId())) {
        return;
    }
    if (dropConnectionLoading(connection)) {
        return;
    }
    dropRegistry(connection);
}

bool AccountManager::dropAccountLoading(const QString &userId)
{
    if (!m_accountsLoading.contains(userId)) {
        return false;
    }

    m_accountsLoading.removeAll(userId);
    Q_EMIT accountsLoadingChanged();
    return true;
}

bool AccountManager::dropConnectionLoading(NeoChatConnection *connection)
{
    if (!connection || (m_connectionsLoading.contains(connection->userId()) && m_connectionsLoading.value(connection->userId(), nullptr) == connection)) {
        return false;
    }

    m_connectionsLoading.remove(connection->userId());
    Quotient::SettingsGroup("Accounts"_L1).remove(connection->userId());
    Q_EMIT connectionLoadingChanged();
    return true;
}

bool AccountManager::dropRegistry(NeoChatConnection *connection)
{
    if (!m_accountRegistry->isLoggedIn(connection->userId())) {
        return false;
    }

    connection->disconnect(this);
    m_accountRegistry->drop(connection);
    Q_EMIT connectionDropped(connection);
    return true;
}
