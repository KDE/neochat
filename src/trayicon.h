// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

// Modified from mujx/nheko's TrayIcon.

#include <QSystemTrayIcon>

/**
 * @class TrayIcon
 *
 * A class inheriting from QSystemTrayIcon to handle setting the system tray icon.
 *
 * Works for Windows, Linux and MacOS.
 *
 * @sa QSystemTrayIcon
 */
class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    TrayIcon(QObject *parent = nullptr);

Q_SIGNALS:
    /**
     * @brief Triggered when the system tray icon is clicked to request NeoChat be shown or hidden.
     */
    void toggleWindow();
};
