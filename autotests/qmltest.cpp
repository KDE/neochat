/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KLocalizedContext>
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

using namespace Quotient;
using namespace Qt::StringLiterals;

// TODO: deduplicate from app/main.cpp
class NetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
    QNetworkAccessManager *create(QObject *) override
    {
        auto nam = NetworkAccessManager::instance();
        nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

        nam->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u"/hsts/"_s);
        nam->setStrictTransportSecurityEnabled(true);

        auto namDiskCache = new QNetworkDiskCache(nam);
        namDiskCache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u"/nam/"_s);
        nam->setCache(namDiskCache);

        return nam;
    }
};

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup() = default;

    Server server;

public Q_SLOTS:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));

        engine->setNetworkAccessManagerFactory(new NetworkAccessManagerFactory());
        engine->rootContext()->setContextObject(new KLocalizedContext(engine));

        server.start();
        Q_UNUSED(new AccountManager(true));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(NeoChat, Setup)

#include "qmltest.moc"