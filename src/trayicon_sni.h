// SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <KStatusNotifierItem>

/**
 * @class TrayIcon
 *
 * A class inheriting KStatusNotifierItem to provide a tray icon.
 *
 * @sa KStatusNotifierItem
 */
class TrayIcon : public KStatusNotifierItem
{
    Q_OBJECT
public:
    explicit TrayIcon(QObject *parent = nullptr);

    /**
     * @brief Show the tray icon.
     */
    void show();

    /**
     * @brief Hide the tray icon.
     */
    void hide();

Q_SIGNALS:
    /**
     * @brief Triggered when the system tray icon is clicked to request NeoChat be shown.
     */
    void showWindow();
};
