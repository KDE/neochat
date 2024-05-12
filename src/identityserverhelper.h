// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

#include <Quotient/jobs/basejob.h>

class NeoChatConnection;

/**
 * @class IdentityServerHelper
 *
 * This class is designed to help the process of setting an identity server for the account.
 * It will manage the various stages of verification and authentication.
 */
class IdentityServerHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The connection to add a 3PID to.
     */
    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The current identity server.
     */
    Q_PROPERTY(QString currentServer READ currentServer NOTIFY currentServerChanged)

    /**
     * @brief Whether an identity server is currently configured.
     */
    Q_PROPERTY(bool hasCurrentServer READ hasCurrentServer NOTIFY currentServerChanged)

    /**
     * @brief The URL for the desired server.
     */
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

    /**
     * @brief The current status.
     */
    Q_PROPERTY(IdServerStatus status READ status NOTIFY statusChanged)

public:
    /**
     * @brief The current status for adding an identity server
     */
    enum IdServerStatus {
        Ready, /**< The process is ready to start. I.e. there is no ongoing attempt to set a new server. */
        Valid, /**< The server URL is valid. */
        Invalid, /**< The server URL is invalid. */
        Match, /**< The server URL is the one that is already configured. */
        Other, /**< An unknown problem occurred. */
    };
    Q_ENUM(IdServerStatus)

    explicit IdentityServerHelper(QObject *parent = nullptr);

    [[nodiscard]] NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

    [[nodiscard]] QString currentServer() const;

    [[nodiscard]] bool hasCurrentServer() const;

    [[nodiscard]] QString url() const;
    void setUrl(const QString &url);

    [[nodiscard]] IdServerStatus status() const;

    /**
     * @brief Set the current URL as the user's identity server.
     *
     * Will do nothing if the URL isn't a valid identity server.
     */
    Q_INVOKABLE void setIdentityServer();

    /**
     * @brief Clear the user's identity server.
     */
    Q_INVOKABLE void clearIdentityServer();

Q_SIGNALS:
    void connectionChanged();
    void currentServerChanged();
    void urlChanged();
    void statusChanged();

private:
    QPointer<NeoChatConnection> m_connection;

    IdServerStatus m_status = Ready;
    QString m_url;

    QPointer<QNetworkReply> m_idServerCheckRequest;

    void checkUrl();
};
