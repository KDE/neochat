// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

#include <Quotient/ssosession.h>

#include "neochatconnection.h"

class HomeserverInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString homeserver READ homeserver WRITE setHomeserver NOTIFY homeserverChanged)
    Q_PROPERTY(bool canSso READ canSso NOTIFY flowsChanged)
    Q_PROPERTY(bool canPassword READ canPassword NOTIFY flowsChanged)
    Q_PROPERTY(QString ssoUrl READ ssoUrl NOTIFY flowsChanged)
    Q_PROPERTY(bool reachable READ reachable NOTIFY reachableChanged)

public:
    explicit HomeserverInfo(QObject *parent = nullptr);

    void setHomeserver(const QString &homeserver);
    QString homeserver() const;

    [[nodiscard]] bool canSso() const;
    [[nodiscard]] bool canPassword() const;
    [[nodiscard]] QString ssoUrl();
    [[nodiscard]] bool reachable() const;

    Q_INVOKABLE void loginWithPassword(const QString &matrixId, const QString &password);

Q_SIGNALS:
    void homeserverChanged();
    void flowsChanged();
    void reachableChanged();

private:
    QString m_homeserver;
    bool m_reachable = false;
    QPointer<NeoChatConnection> m_testConnection;
    QPointer<Quotient::SsoSession> m_ssoSession;

    void test();
    void setReachable(bool reachable);
};
