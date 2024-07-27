// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QQmlEngine>

#include <Quotient/accountregistry.h>
#include <Quotient/e2ee/sssshandler.h>
#include <Quotient/keyverificationsession.h>
#include <Quotient/roommember.h>

#include "controller.h"
#include "neochatconfig.h"

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

struct ForeignSSSSHandler {
    Q_GADGET
    QML_FOREIGN(Quotient::SSSSHandler)
    QML_NAMED_ELEMENT(SSSSHandler)
};
