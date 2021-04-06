// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

// Modified from mujx/nheko's TrayIcon.

#include <QAction>
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QRect>
#include <QSystemTrayIcon>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    TrayIcon(QObject *parent = nullptr);

Q_SIGNALS:
    void showWindow();
};
