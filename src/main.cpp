// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCommandLineParser>
#include <QIcon>
#include <QNetworkDiskCache>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlNetworkAccessManagerFactory>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtQml/QQmlExtensionPlugin>
#include <Quotient/connection.h>

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

#if __has_include("KCrash")
#include <KCrash>
#endif

#include <KIconTheme>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "neochat-version.h"

#include <Quotient/networkaccessmanager.h>

#include "blurhashimageprovider.h"
#include "colorschemer.h"
#include "controller.h"
#include "logger.h"
#include "roommanager.h"
#include "sentryintegration.h"
#include "sharehandler.h"
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

        nam->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u"/hsts/"_s);
        nam->setStrictTransportSecurityEnabled(true);

        auto namDiskCache = new QNetworkDiskCache(nam);
        namDiskCache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u"/nam/"_s);
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
    KIconTheme::initTheme();
    QNetworkProxyFactory::setUseSystemConfiguration(true);

#ifdef HAVE_WEBVIEW
    QtWebView::initialize();
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
#endif

#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(u"org.kde.breeze"_s);
#else
    QIcon::setFallbackThemeName("breeze"_L1);
    QApplication app(argc, argv);
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(u"org.kde.desktop"_s);
    }
#endif

#ifdef Q_OS_WINDOWS
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    QApplication::setStyle(u"breeze"_s);
    QFont font(u"Segoe UI Emoji"_s);
    font.setPointSize(10);
    font.setHintingPreference(QFont::PreferNoHinting);
    app.setFont(font);
#endif
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));

    QGuiApplication::setOrganizationName("KDE"_L1);

    KAboutData about(u"neochat"_s,
                     i18n("NeoChat"),
                     QStringLiteral(NEOCHAT_VERSION_STRING),
                     i18n("Chat on Matrix"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2018-2020 Black Hat, 2020-2024 KDE Community"));
    about.addAuthor(i18n("Carl Schwan"), i18n("Maintainer"), u"carl@carlschwan.eu"_s, u"https://carlschwan.eu"_s, QUrl(u"https://carlschwan.eu/avatar.png"_s));
    about.addAuthor(i18n("Tobias Fella"), i18n("Maintainer"), u"tobias.fella@kde.org"_s, u"https://tobiasfella.de"_s);
    about.addAuthor(i18n("James Graham"), i18n("Maintainer"), u"james.h.graham@protonmail.com"_s);
    about.addAuthor(i18n("Joshua Goins"),
                    i18n("Maintainer"),
                    u"josh@redstrate.com"_s,
                    u"https://redstrate.com/"_s,
                    QUrl(u"https://redstrate.com/rss-image.png"_s));
    about.addCredit(i18n("Black Hat"), i18n("Original author of Spectral"), u"bhat@encom.eu.org"_s);
    about.addCredit(i18n("Alexey Rusakov"), i18n("Maintainer of libQuotient"), u"Kitsune-Ral@users.sf.net"_s);
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    about.setOrganizationDomain("kde.org");

    about.addComponent(u"libQuotient"_s,
                       i18n("A Qt library to write cross-platform clients for Matrix"),
                       i18nc("<version number> (built against <possibly different version number>)",
                             "%1 (built against %2)",
                             Quotient::versionString(),
                             QStringLiteral(Quotient_VERSION_STRING)),
                       u"https://github.com/quotient-im/libquotient"_s,
                       KAboutLicense::LGPL_V2_1);

    KAboutData::setApplicationData(about);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"org.kde.neochat"_s));

#if __has_include("KCrash")
    KCrash::initialize();
#endif

    Sentry::instance();

    initLogging();

    Connection::setEncryptionDefault(true);
    Connection::setDirectChatEncryptionDefault(true);

#ifdef NEOCHAT_FLATPAK
    // Copy over the included FontConfig configuration to the
    // app's config dir:
    QFile::copy(u"/app/etc/fonts/conf.d/99-noto-mono-color-emoji.conf"_s, u"/var/config/fontconfig/conf.d/99-noto-mono-color-emoji.conf"_s);
#endif

    ColorSchemer colorScheme;

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Client for the matrix communication protocol"));
    parser.addPositionalArgument(u"urls"_s, i18n("Supports matrix: url scheme"));
    parser.addOption(QCommandLineOption("ignore-ssl-errors"_L1, i18n("Ignore all SSL Errors, e.g., unsigned certificates.")));

    QCommandLineOption testOption("test"_L1, i18n("Only used for autotests"));
    testOption.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(testOption);

#ifdef HAVE_KUNIFIEDPUSH
    QCommandLineOption dbusActivatedOption(u"dbus-activated"_s, i18n("Internal usage only."));
    dbusActivatedOption.setFlags(QCommandLineOption::Flag::HiddenFromHelp);
    parser.addOption(dbusActivatedOption);
#endif

    QCommandLineOption shareOption(u"share"_s, i18n("Share a URL to Matrix"), u"text"_s);
    parser.addOption(shareOption);

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
    Controller::setTestMode(parser.isSet("test"_L1));

#ifdef HAVE_KUNIFIEDPUSH
    if (parser.isSet(dbusActivatedOption)) {
        // We want to be replaceable by the main client
        KDBusService service(KDBusService::Replace);

#ifdef HAVE_RUNNER
        // If we are built with KRunner and KUnifiedPush support, we need to do something special.
        // Because KRunner may call us on the D-Bus (under the same service name org.kde.neochat) then it may
        // accidentally activate us for push notifications instead. If this happens, then immediately quit if the fake
        // runner is called.
        QDBusConnection::sessionBus().registerObject("/RoomRunner"_L1, new FakeRunner(), QDBusConnection::ExportScriptableContents);
#endif

        Controller::listenForNotifications();
        return QCoreApplication::exec();
    }
#endif

#ifdef HAVE_KDBUSADDONS
    KDBusService service(KDBusService::Unique);
#endif

    Q_IMPORT_QML_PLUGIN(org_kde_neochat_settingsPlugin)
    Q_IMPORT_QML_PLUGIN(org_kde_neochat_timelinePlugin)
    Q_IMPORT_QML_PLUGIN(org_kde_neochat_devtoolsPlugin)
    Q_IMPORT_QML_PLUGIN(org_kde_neochat_loginPlugin)
    Q_IMPORT_QML_PLUGIN(org_kde_neochat_chatbarPlugin)

    qml_register_types_org_kde_neochat();

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
            if (args.length() == 2 && args[0] == "--share"_L1) {
                ShareHandler::instance().setText(args[1]);
                return;
            }

            for (const auto &arg : args) {
                RoomManager::instance().resolveResource(arg);
            }
        });
#endif

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.setNetworkAccessManagerFactory(new NetworkAccessManagerFactory());

    if (parser.isSet("ignore-ssl-errors"_L1)) {
        QObject::connect(NetworkAccessManager::instance(), &QNetworkAccessManager::sslErrors, NetworkAccessManager::instance(), [](QNetworkReply *reply) {
            reply->ignoreSslErrors();
        });
    }

    if (parser.isSet("share"_L1)) {
        ShareHandler::instance().setText(parser.value(shareOption));
    }

    engine.addImageProvider(u"blurhash"_s, new BlurhashImageProvider);

    engine.loadFromModule("org.kde.neochat", "Main");
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (!parser.positionalArguments().isEmpty() && !parser.isSet("share"_L1)) {
        RoomManager::instance().setUrlArgument(parser.positionalArguments()[0]);
    }

#ifdef HAVE_RUNNER
    auto runner = Runner::create(&engine, &engine);
    QDBusConnection::sessionBus().registerObject("/RoomRunner"_L1, runner, QDBusConnection::ExportScriptableContents);
#endif

    QWindow *window = windowFromEngine(&engine);

    WindowController::instance().setWindow(window);

    return app.exec();
}
