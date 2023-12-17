// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCommandLineParser>
#include <QIcon>
#include <QNetworkDiskCache>
#include <QNetworkProxyFactory>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlNetworkAccessManagerFactory>
#include <QQuickStyle>
#include <QQuickWindow>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#ifdef HAVE_WEBVIEW
#include <QtWebView>
#endif

#include <KAboutData>
#ifdef HAVE_KDBUSADDONS
#include <KDBusService>
#endif
#ifdef HAVE_WINDOWSYSTEM
#include <KWindowSystem>
#endif
#include <KLocalizedContext>
#include <KLocalizedString>

#include "neochat-version.h"

#include <Quotient/networkaccessmanager.h>

#include "blurhashimageprovider.h"
#include "colorschemer.h"
#include "controller.h"
#include "logger.h"
#include "matriximageprovider.h"
#include "neochatconfig.h"
#include "roommanager.h"
#include "windowcontroller.h"

#ifdef HAVE_RUNNER
#include "runner.h"
#include <QDBusConnection>
#include <QDBusMetaType>
#endif

#if defined(HAVE_RUNNER) && defined(HAVE_KUNIFIEDPUSH)
#include "fakerunner.h"
#endif

#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif

using namespace Quotient;

void qml_register_types_org_kde_neochat();

class NetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
    QNetworkAccessManager *create(QObject *) override
    {
        auto nam = NetworkAccessManager::instance();
        nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

        nam->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/hsts/"));
        nam->setStrictTransportSecurityEnabled(true);

        auto namDiskCache = new QNetworkDiskCache(nam);
        namDiskCache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/nam/"));
        nam->setCache(namDiskCache);

        return nam;
    }
};

static QWindow *windowFromEngine(QQmlApplicationEngine *engine)
{
    const auto rootObjects = engine->rootObjects();
    auto *window = qobject_cast<QQuickWindow *>(rootObjects.first());
    Q_ASSERT(window);
    return window;
}

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char *argv[])
{
    QNetworkProxyFactory::setUseSystemConfiguration(true);

#ifdef HAVE_WEBVIEW
    QtWebView::initialize();
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
#endif

#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
#else
    QIcon::setFallbackThemeName("breeze"_ls);
    QApplication app(argc, argv);
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif

#ifdef Q_OS_WINDOWS
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    QApplication::setStyle(QStringLiteral("breeze"));
    QFont font(QStringLiteral("Segoe UI Emoji"));
    font.setPointSize(10);
    font.setHintingPreference(QFont::PreferNoHinting);
    app.setFont(font);
#endif
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));

    QGuiApplication::setOrganizationName("KDE"_ls);

    KAboutData about(QStringLiteral("neochat"),
                     i18n("NeoChat"),
                     QStringLiteral(NEOCHAT_VERSION_STRING),
                     i18n("Matrix client"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2018-2020 Black Hat, 2020-2024 KDE Community"));
    about.addAuthor(i18n("Carl Schwan"),
                    i18n("Maintainer"),
                    QStringLiteral("carl@carlschwan.eu"),
                    QStringLiteral("https://carlschwan.eu"),
                    QStringLiteral("https://carlschwan.eu/avatar.png"));
    about.addAuthor(i18n("Tobias Fella"), i18n("Maintainer"), QStringLiteral("tobias.fella@kde.org"), QStringLiteral("https://tobiasfella.de"));
    about.addAuthor(i18n("James Graham"), i18n("Maintainer"), QStringLiteral("james.h.graham@protonmail.com"));
    about.addCredit(i18n("Black Hat"), i18n("Original author of Spectral"), QStringLiteral("bhat@encom.eu.org"));
    about.addCredit(i18n("Alexey Rusakov"), i18n("Maintainer of Quotient"), QStringLiteral("Kitsune-Ral@users.sf.net"));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    about.setOrganizationDomain("kde.org");

    about.addComponent(QStringLiteral("libQuotient"),
                       i18n("A Qt library to write cross-platform clients for Matrix"),
                       i18nc("<version number> (built against <possibly different version number>)",
                             "%1 (built against %2)",
                             Quotient::versionString(),
                             QStringLiteral(Quotient_VERSION_STRING)),
                       QStringLiteral("https://github.com/quotient-im/libquotient"),
                       KAboutLicense::LGPL_V2_1);

    KAboutData::setApplicationData(about);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.neochat")));

    initLogging();

    Connection::setEncryptionDefault(true);

#ifdef NEOCHAT_FLATPAK
    // Copy over the included FontConfig configuration to the
    // app's config dir:
    QFile::copy(QStringLiteral("/app/etc/fonts/conf.d/99-noto-mono-color-emoji.conf"),
                QStringLiteral("/var/config/fontconfig/conf.d/99-noto-mono-color-emoji.conf"));
#endif

    ColorSchemer colorScheme;
    if (!NeoChatConfig::self()->colorScheme().isEmpty()) {
        colorScheme.apply(NeoChatConfig::self()->colorScheme());
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Client for the matrix communication protocol"));
    parser.addPositionalArgument(QStringLiteral("urls"), i18n("Supports matrix: url scheme"));
    parser.addOption(QCommandLineOption("ignore-ssl-errors"_ls, i18n("Ignore all SSL Errors, e.g., unsigned certificates.")));

    QCommandLineOption testOption("test"_ls, i18n("Only used for autotests"));
    testOption.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(testOption);

#ifdef HAVE_KUNIFIEDPUSH
    QCommandLineOption dbusActivatedOption(QStringLiteral("dbus-activated"), i18n("Internal usage only."));
    dbusActivatedOption.setFlags(QCommandLineOption::Flag::HiddenFromHelp);
    parser.addOption(dbusActivatedOption);
#endif

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
    Controller::setTestMode(parser.isSet("test"_ls));

#ifdef HAVE_KUNIFIEDPUSH
    if (parser.isSet(dbusActivatedOption)) {
        // We want to be replaceable by the main client
        KDBusService service(KDBusService::Replace);

#ifdef HAVE_RUNNER
        // If we are built with KRunner and KUnifiedPush support, we need to do something special.
        // Because KRunner may call us on the D-Bus (under the same service name org.kde.neochat) then it may
        // accidentally activate us for push notifications instead. If this happens, then immediately quit if the fake
        // runner is called.
        QDBusConnection::sessionBus().registerObject("/RoomRunner"_ls, new FakeRunner(), QDBusConnection::ExportScriptableContents);
#endif

        Controller::listenForNotifications();
        return QCoreApplication::exec();
    }
#endif

#ifdef HAVE_KDBUSADDONS
    KDBusService service(KDBusService::Unique);
#endif

    qml_register_types_org_kde_neochat();
    qmlRegisterSingletonInstance("org.kde.neochat.config", 1, 0, "Config", NeoChatConfig::self());
    qmlRegisterSingletonInstance("org.kde.neochat.accounts", 1, 0, "AccountRegistry", &Controller::instance().accounts());

    qmlRegisterUncreatableType<KeyVerificationSession>("com.github.quotient_im.libquotient", 1, 0, "KeyVerificationSession", {});

    QQmlApplicationEngine engine;

#ifdef HAVE_KDBUSADDONS
    service.connect(&service,
                    &KDBusService::activateRequested,
                    &RoomManager::instance(),
                    [&engine](const QStringList &arguments, const QString &workingDirectory) {
                        Q_UNUSED(workingDirectory);

                        QWindow *window = windowFromEngine(&engine);
                        KWindowSystem::updateStartupId(window);

                        WindowController::instance().showAndRaiseWindow(QString());

                        // Open matrix uri
                        if (arguments.isEmpty()) {
                            return;
                        }
                        auto args = arguments;
                        args.removeFirst();
                        for (const auto &arg : args) {
                            RoomManager::instance().resolveResource(arg);
                        }
                    });
#endif

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);
    engine.setNetworkAccessManagerFactory(new NetworkAccessManagerFactory());

    if (parser.isSet("ignore-ssl-errors"_ls)) {
        QObject::connect(NetworkAccessManager::instance(), &QNetworkAccessManager::sslErrors, NetworkAccessManager::instance(), [](QNetworkReply *reply) {
            reply->ignoreSslErrors();
        });
    }

    engine.addImageProvider(QLatin1String("mxc"), MatrixImageProvider::create(&engine, &engine));
    engine.addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

    engine.load(QUrl(QStringLiteral("qrc:/org/kde/neochat/qml/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (!parser.positionalArguments().isEmpty()) {
        RoomManager::instance().setUrlArgument(parser.positionalArguments()[0]);
    }

#ifdef HAVE_RUNNER
    auto runner = Runner::create(&engine, &engine);
    QDBusConnection::sessionBus().registerObject("/RoomRunner"_ls, runner, QDBusConnection::ExportScriptableContents);
#endif

    QWindow *window = windowFromEngine(&engine);

    WindowController::instance().setWindow(window);
    WindowController::instance().restoreGeometry();

    return app.exec();
}
