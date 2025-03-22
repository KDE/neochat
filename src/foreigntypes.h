// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QQmlEngine>

#include <Integral/Accounts>
#include <Integral/Homeserver>

#include "controller.h"
#include "neochatconfig.h"

struct ForeignAccounts {
    Q_GADGET
    QML_ELEMENT
    QML_SINGLETON
    QML_FOREIGN(Integral::Accounts)
    QML_NAMED_ELEMENT(Accounts)
    static Integral::Accounts *create(QQmlEngine *, QJSEngine *)
    {
        auto &accounts = Controller::instance().accounts();
        QQmlEngine::setObjectOwnership(&accounts, QQmlEngine::CppOwnership);
        return &accounts;
    }
};

struct ForeignHomeserver {
    Q_GADGET
    QML_ELEMENT
    QML_FOREIGN(Integral::Homeserver)
    QML_NAMED_ELEMENT(Homeserver)
};

struct ForeignConnection {
    Q_GADGET
    QML_ELEMENT
    QML_FOREIGN(Integral::Connection)
    QML_NAMED_ELEMENT(Connection)
};
