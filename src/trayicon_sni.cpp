// SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "trayicon_sni.h"
#include <KWindowSystem>

#include "windowcontroller.h"

TrayIcon::TrayIcon(QObject *parent)
    : KStatusNotifierItem(parent)
{
    setCategory(KStatusNotifierItem::ItemCategory::Communications);
    setIconByName(QStringLiteral("org.kde.neochat.tray"));
    connect(this, &KStatusNotifierItem::activateRequested, this, [this] {
        KWindowSystem::setCurrentXdgActivationToken(providedToken());
        WindowController::instance().toggleWindow();
    });

    connect(&WindowController::instance(), &WindowController::windowChanged, this, [this] {
        setAssociatedWindow(WindowController::instance().window());
    });
}

void TrayIcon::show()
{
    setStatus(Active);
}

void TrayIcon::hide()
{
    setStatus(Passive);
}

#include "moc_trayicon_sni.cpp"
