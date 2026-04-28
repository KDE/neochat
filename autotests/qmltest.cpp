/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KLocalizedString>
#include <QNetworkDiskCache>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlNetworkAccessManagerFactory>
#include <QStandardPaths>
#include <Quotient/networkaccessmanager.h>
#include <quicktest.h>

#include "accountmanager.h"
#include "server.h"

#include <KLocalizedQmlContext>

using namespace Quotient;
using namespace Qt::StringLiterals;

class NetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
    QNetworkAccessManager *create(QObject *) override
    {
        auto nam = NetworkAccessManager::instance();
        QObject::connect(nam, &QNetworkAccessManager::sslErrors, nam, [](auto reply, auto errors) {
            reply->ignoreSslErrors(errors);
        });
        nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

        return nam;
    }
};

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup() = default;

public Q_SLOTS:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));

        Server::instance()->start();

        engine->setNetworkAccessManagerFactory(new NetworkAccessManagerFactory());
        engine->rootContext()->setContextObject(new KLocalizedQmlContext(engine));

        const QString databaseFolder{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) % u"/@user_localhost_1234"_s};
        QDir(databaseFolder).removeRecursively();
        auto accountManager = new AccountManager(true);
        engine->rootContext()->setContextProperty(u"accountManager"_s, accountManager);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(NeoChat, Setup)

#include "qmltest.moc"
