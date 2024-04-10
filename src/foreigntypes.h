// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QQmlEngine>

#include <Quotient/accountregistry.h>
#include <Quotient/keyverificationsession.h>
#if __has_include("Quotient/e2ee/sssshandler.h")
#include <Quotient/e2ee/sssshandler.h>
#endif

#include "controller.h"
#include "neochatconfig.h"

struct ForeignConfig {
    Q_GADGET
    QML_FOREIGN(NeoChatConfig)
    QML_NAMED_ELEMENT(Config)
    QML_SINGLETON
public:
    static NeoChatConfig *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(NeoChatConfig::self(), QQmlEngine::CppOwnership);
        return NeoChatConfig::self();
    }
};

struct ForeignAccountRegistry {
    Q_GADGET
    QML_FOREIGN(Quotient::AccountRegistry)
    QML_NAMED_ELEMENT(AccountRegistry)
    QML_SINGLETON
public:
    static Quotient::AccountRegistry *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(&Controller::instance().accounts(), QQmlEngine::CppOwnership);
        return &Controller::instance().accounts();
    }
};

struct ForeignKeyVerificationSession {
    Q_GADGET
    QML_FOREIGN(Quotient::KeyVerificationSession)
    QML_NAMED_ELEMENT(KeyVerificationSession)
    QML_UNCREATABLE("")
};

#if __has_include("Quotient/e2ee/sssshandler.h")
struct ForeignSSSSHandler {
    Q_GADGET
    QML_FOREIGN(Quotient::SSSSHandler)
    QML_NAMED_ELEMENT(SSSSHandler)
};
#endif
