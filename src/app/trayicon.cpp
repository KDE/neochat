// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-3.0-only

#include "trayicon.h"

#include <QCoreApplication>
#include <QMenu>

#include <KLocalizedString>

#include "windowcontroller.h"

using namespace Qt::StringLiterals;

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
    , m_menu(std::make_unique<QMenu>())
{
    setIcon(QIcon::fromTheme(u"org.kde.neochat.tray"_s));
    auto viewAction_ = new QAction(i18nc("@action:inmenu Show the NeoChat window", "Show"), m_menu.get());

    connect(viewAction_, &QAction::triggered, this, [] {
        WindowController::instance().toggleWindow();
    });
    connect(this, &QSystemTrayIcon::activated, this, [](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            WindowController::instance().toggleWindow();
        }
    });

    m_menu->addAction(viewAction_);

    m_menu->addSeparator();

    auto quitAction = new QAction(i18nc("@action:inmenu Quit NeoChat", "Quit"), m_menu.get());
    quitAction->setIcon(QIcon::fromTheme(u"application-exit"_s));
    connect(quitAction, &QAction::triggered, QCoreApplication::instance(), QCoreApplication::quit);

    m_menu->addAction(quitAction);

    setContextMenu(m_menu.get());
}

#include "moc_trayicon.cpp"
