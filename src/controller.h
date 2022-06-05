// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "models/pushrulemodel.h"
#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>

#include <KFormat>

#include "neochatconnection.h"
#include <Quotient/accountregistry.h>
#include <Quotient/jobs/basejob.h>
#include <Quotient/settings.h>

#ifdef HAVE_KUNIFIEDPUSH
#include <qcoro/task.h>
#endif

class NeoChatRoom;
class TrayIcon;
class QQuickTextDocument;

namespace Quotient
{
class Room;
class User;
}

namespace QKeychain
{
class ReadPasswordJob;
}

/**
 * @class Controller
 *
 * A singleton class designed to help manage the application.
 *
 * There are also a bunch of helper functions that currently don't fit anywhere
 * else.
 */
class Controller : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /**
     * @brief The current connection for the rest of NeoChat to use.
     */
    Q_PROPERTY(NeoChatConnection *activeConnection READ activeConnection WRITE setActiveConnection NOTIFY activeConnectionChanged)

    /**
     * @brief The PushRuleModel that has the active connection's push rules.
     */
    Q_PROPERTY(PushRuleModel *pushRuleModel READ pushRuleModel CONSTANT)

    /**
     * @brief The row number in the accounts directory of the active connection.
     */
    Q_PROPERTY(int activeConnectionIndex READ activeConnectionIndex NOTIFY activeConnectionIndexChanged)

    /**
     * @brief Whether the OS NeoChat is running on supports sytem tray icons.
     */
    Q_PROPERTY(bool supportSystemTray READ supportSystemTray CONSTANT)

    /**
     * @brief Whether KWindowSystem specific features are available.
     */
    Q_PROPERTY(bool hasWindowSystem READ hasWindowSystem CONSTANT)

    /**
     * @brief Whether NeoChat is currently able to connect to the server.
     */
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)

    /**
     * @brief Whether NeoChat is running as a flatpak.
     *
     * This is the only way to gate NeoChat features in flatpaks in QML.
     */
    Q_PROPERTY(bool isFlatpak READ isFlatpak CONSTANT)

public:
    /**
     * @brief Defines the status after an attempt to change the password on an account.
     */
    enum PasswordStatus {
        Success, /**< The password was successfully changed. */
        Wrong, /**< The current password entered was wrong. */
        Other, /**< An unknown problem occurred. */
    };
    Q_ENUM(PasswordStatus)

    static Controller &instance();
    static Controller *create(QQmlEngine *, QJSEngine *)
    {
        return &instance();
    }

    void setActiveConnection(NeoChatConnection *connection);
    [[nodiscard]] NeoChatConnection *activeConnection() const;

    [[nodiscard]] PushRuleModel *pushRuleModel() const;

    /**
     * @brief Add a new connection to the account registry.
     */
    void addConnection(NeoChatConnection *c);

    /**
     * @brief Drop a connection from the account registry.
     */
    void dropConnection(NeoChatConnection *c);

    int activeConnectionIndex() const;

    /**
     * @brief Save an access token to the keychain for the given account.
     */
    bool saveAccessTokenToKeyChain(const Quotient::AccountSettings &account, const QByteArray &accessToken);

    /**
     * @brief Join a room.
     */
    Q_INVOKABLE void joinRoom(const QString &alias);

    /**
     * @brief Join a direct chat with the given user.
     *
     * If a direct chat with the user doesn't exist one is created and then joined.
     */
    Q_INVOKABLE void openOrCreateDirectChat(Quotient::User *user);

    [[nodiscard]] bool supportSystemTray() const;

    /**
     * @brief Set the background blur status of the given item.
     */
    Q_INVOKABLE void setBlur(QQuickItem *item, bool blur);

    bool isOnline() const;

    /**
     * @brief Sets the QNetworkProxy for the application.
     *
     * @sa QNetworkProxy::setApplicationProxy
     */
    Q_INVOKABLE void setApplicationProxy();

    bool isFlatpak() const;

    /**
     * @brief Return a string for the input timestamp.
     *
     * The output format depends on the KFormat::DurationFormatOptions chosen.
     *
     * @sa KFormat::DurationFormatOptions
     */
    Q_INVOKABLE QString formatDuration(quint64 msecs, KFormat::DurationFormatOptions options = KFormat::DefaultDuration) const;

    /**
     * @brief Return a human readable string for a given input number of bytes.
     */
    Q_INVOKABLE QString formatByteSize(double size, int precision = 1) const;

    /**
     * @brief Force a QQuickTextDocument to refresh when images are loaded.
     *
     * HACK: This is a workaround for QTBUG 93281.
     */
    Q_INVOKABLE void forceRefreshTextDocument(QQuickTextDocument *textDocument, QQuickItem *item);

    Quotient::AccountRegistry &accounts();

private:
    explicit Controller(QObject *parent = nullptr);

    QPointer<NeoChatConnection> m_connection;

#ifdef HAVE_KUNIFIEDPUSH
    QCoro::Task<void> setupPush(const QString &endpoint);
#endif

    TrayIcon *m_trayIcon = nullptr;

    QKeychain::ReadPasswordJob *loadAccessTokenFromKeyChain(const Quotient::AccountSettings &account);

    void loadSettings();
    void saveSettings() const;
    bool m_isOnline = true;
    QMap<Quotient::Room *, int> m_notificationCounts;

    bool hasWindowSystem() const;

    QPointer<PushRuleModel> m_pushRuleModel;
    Quotient::AccountRegistry m_accountRegistry;

private Q_SLOTS:
    void invokeLogin();
    void toggleWindow();
    void setQuitOnLastWindowClosed();

Q_SIGNALS:
    /// Error occurred because of user inputs
    void errorOccured(const QString &error);

    /// Error occurred because of server or bug in NeoChat
    void globalErrorOccured(QString error, QString detail);
    void syncDone();
    void connectionAdded(NeoChatConnection *connection);
    void connectionDropped(NeoChatConnection *connection);
    void initiated();
    void quitOnLastWindowClosedChanged();
    void unreadCountChanged();
    void activeConnectionChanged();
    void passwordStatus(Controller::PasswordStatus status);
    void userConsentRequired(QUrl url);
    void isOnlineChanged(bool isOnline);
    void activeConnectionIndexChanged();

public Q_SLOTS:
    void saveWindowGeometry();
};
