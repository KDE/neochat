// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QQmlEngine>

#include <QuotientNg/Accounts>
#include <QuotientNg/Homeserver>

#include "controller.h"
#include "neochatconfig.h"

struct ForeignAccounts {
    Q_GADGET
    QML_ELEMENT
    QML_SINGLETON
    QML_FOREIGN(Quotient::Accounts)
    QML_NAMED_ELEMENT(Accounts)
    static Quotient::Accounts *create(QQmlEngine *engine, QJSEngine *)
    {
        auto &accounts = Controller::instance().accounts();
        QQmlEngine::setObjectOwnership(&accounts, QQmlEngine::CppOwnership);
        return &accounts;
    }
};

struct ForeignHomeserver {
    Q_GADGET
    QML_ELEMENT
    QML_FOREIGN(Quotient::Homeserver)
    QML_NAMED_ELEMENT(Homeserver)
};
