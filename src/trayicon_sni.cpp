// SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "trayicon_sni.h"
#include <KWindowSystem>

#include "windowcontroller.h"

using namespace Qt::StringLiterals;

TrayIcon::TrayIcon(QObject *parent)
    : KStatusNotifierItem(parent)
{
    setCategory(KStatusNotifierItem::ItemCategory::Communications);
    setIconByName(u"org.kde.neochat.tray"_s);

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
