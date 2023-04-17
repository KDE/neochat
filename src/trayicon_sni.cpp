// SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "trayicon_sni.h"
#include <KWindowSystem>

#include "windowcontroller.h"

TrayIcon::TrayIcon(QObject *parent)
    : KStatusNotifierItem(parent)
{
    setCategory(KStatusNotifierItem::ItemCategory::Communications);
    setIconByName("org.kde.neochat.tray");
    connect(this, &KStatusNotifierItem::activateRequested, this, [this] {
        KWindowSystem::setCurrentXdgActivationToken(providedToken());
        Q_EMIT showWindow();
    });

    connect(&WindowController::instance(), &WindowController::windowChanged, this, [this] {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        setAssociatedWindow(WindowController::instance().window());
#endif
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
