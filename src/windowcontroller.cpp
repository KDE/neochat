// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "windowcontroller.h"

#include <KConfig>
#include <KWindowConfig>

#ifdef HAVE_WINDOWSYSTEM
#include <KWindowSystem>
#endif

#include <QStandardPaths>

WindowController &WindowController::instance()
{
    static WindowController instance;
    return instance;
}

void WindowController::setWindow(QWindow *window)
{
    m_window = window;
}

void WindowController::restoreGeometry()
{
    KConfig dataResource("data", KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
    KConfigGroup windowGroup(&dataResource, "Window");
    KWindowConfig::restoreWindowSize(m_window, windowGroup);
    KWindowConfig::restoreWindowPosition(m_window, windowGroup);
}

void WindowController::saveGeometry()
{
    KConfig dataResource("data", KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
    KConfigGroup windowGroup(&dataResource, "Window");
    KWindowConfig::saveWindowPosition(m_window, windowGroup);
    KWindowConfig::saveWindowSize(m_window, windowGroup);
}

void WindowController::showAndRaiseWindow(const QString &xdgActivationToken)
{
    if (!m_window->isVisible()) {
        m_window->show();
    }

#ifdef HAVE_WINDOWSYSTEM
    if (!xdgActivationToken.isEmpty()) {
        KWindowSystem::setCurrentXdgActivationToken(xdgActivationToken);
    }

    KWindowSystem::activateWindow(m_window);
#endif
}
