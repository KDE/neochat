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

public:
    explicit HomeserverInfo(QObject *parent = nullptr);

    void setHomeserver(const QString &homeserver);
    QString homeserver() const;

    [[nodiscard]] bool canSso() const;
    [[nodiscard]] bool canPassword() const;
    [[nodiscard]] QString ssoUrl();
    ;

Q_SIGNALS:
    void homeserverChanged();
    void flowsChanged();

private:
    QString m_homeserver;
    void test();
    QPointer<NeoChatConnection> m_testConnection;
    QPointer<Quotient::SsoSession> m_ssoSession;
};
