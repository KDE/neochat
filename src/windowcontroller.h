// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QWindow>

/**
 * @class WindowController
 *
 * A singleton class to help manage the NeoChat window.
 */
class WindowController : public QObject
{
    Q_OBJECT

public:
    static WindowController &instance();

    /**
     * @brief Set the window that the will be managed.
     */
    void setWindow(QWindow *window);

    /**
     * @brief Get the window that the will be managed.
     */
    QWindow *window() const;

    /**
     * @brief Restore any saved window geometry if available.
     */
    void restoreGeometry();

    /**
     * @brief Save the current window geometry.
     */
    void saveGeometry();

    /**
     * @brief Show the window and raise to the top.
     */
    void showAndRaiseWindow(const QString &startupId);

Q_SIGNALS:
    /**
     * @brief Triggered if the managed window is changed.
     */
    void windowChanged();

private:
    WindowController() = default;

    QWindow *m_window = nullptr;
};
