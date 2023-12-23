// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-3.0-only

#include "trayicon.h"

#include <QCoreApplication>
#include <QMenu>

#include <KLocalizedString>

#include "windowcontroller.h"

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
{
    setIcon(QIcon(QStringLiteral(":/icons/org.kde.neochat.tray.svg")));
    QMenu *menu = new QMenu();
    auto viewAction_ = new QAction(i18n("Show"), parent);

    connect(viewAction_, &QAction::triggered, this, [] {
        WindowController::instance().toggleWindow();
    });
    connect(this, &QSystemTrayIcon::activated, this, [](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            WindowController::instance().toggleWindow();
        }
    });

    menu->addAction(viewAction_);

    menu->addSeparator();

    auto quitAction = new QAction(i18n("Quit"), parent);
    quitAction->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));
    connect(quitAction, &QAction::triggered, QCoreApplication::instance(), QCoreApplication::quit);

    menu->addAction(quitAction);

    setContextMenu(menu);
}

#include "moc_trayicon.cpp"
