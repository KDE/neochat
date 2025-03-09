// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QQmlEngine>

#include <QuotientNg/Accounts>

#include "controller.h"
#include "neochatconfig.h"

struct ForeignKeyImport {
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
