/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef TRAYICON_H
#define TRAYICON_H

// Modified from mujx/nheko's TrayIcon.

#include <QSystemTrayIcon>
#include <QAction>
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QRect>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    TrayIcon(QObject *parent = nullptr);

Q_SIGNALS:
    void showWindow();
};

#endif // TRAYICON_H
