// SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "trayicon_sni.h"
#include <KWindowSystem>

TrayIcon::TrayIcon(QObject *parent)
    : KStatusNotifierItem(parent)
{
    setIconByName("org.kde.neochat");
    connect(this, &KStatusNotifierItem::activateRequested, this, [this] {
        KWindowSystem::setCurrentXdgActivationToken(providedToken());
        Q_EMIT showWindow();
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
