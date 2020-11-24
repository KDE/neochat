/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "trayicon.h"

#include <QCoreApplication>
#include <QMenu>

#include <KLocalizedString>

TrayIcon::TrayIcon(QObject *parent)
    : KStatusNotifierItem(parent)
{
    QMenu *menu = new QMenu();
    auto viewAction_ = new QAction(i18n("Show"), parent);

    connect(viewAction_, &QAction::triggered, this, &TrayIcon::showWindow);
    connect(this, &KStatusNotifierItem::activateRequested, this, [this] (bool active) {
        if (active) {
            Q_EMIT showWindow();
        }
    });

    menu->addAction(viewAction_);

    setCategory(Communications);
    setContextMenu(menu);
}

void TrayIcon::setIsOnline(bool online)
{
    m_isOnline = online;
    setStatus(Active);
    Q_EMIT isOnlineChanged();
}

void TrayIcon::setIconSource(const QString &source)
{
    m_iconSource = source;
    setIconByName(source);
    Q_EMIT iconSourceChanged();
}
