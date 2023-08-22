// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "windowcontroller.h"

#include <KConfig>
#include <KWindowConfig>

#ifdef HAVE_WINDOWSYSTEM
#if HAVE_X11
#include <KStartupInfo>
#endif
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

    Q_EMIT windowChanged();
}

QWindow *WindowController::window() const
{
    return m_window;
}

void WindowController::restoreGeometry()
{
    KConfig dataResource(QStringLiteral("data"), KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
    KConfigGroup windowGroup(&dataResource, QStringLiteral("Window"));
    KWindowConfig::restoreWindowSize(m_window, windowGroup);
    KWindowConfig::restoreWindowPosition(m_window, windowGroup);
}

void WindowController::saveGeometry()
{
    KConfig dataResource(QStringLiteral("data"), KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
    KConfigGroup windowGroup(&dataResource, QStringLiteral("Window"));
    KWindowConfig::saveWindowPosition(m_window, windowGroup);
    KWindowConfig::saveWindowSize(m_window, windowGroup);
}

void WindowController::showAndRaiseWindow(const QString &startupId)
{
    if (!m_window->isVisible()) {
        m_window->show();
        restoreGeometry();
    }

#ifdef HAVE_WINDOWSYSTEM
    if (!startupId.isEmpty()) {
        if (KWindowSystem::isPlatformX11()) {
#if HAVE_X11
            KStartupInfo::setNewStartupId(m_window, startupId.toUtf8());
#endif
        } else if (KWindowSystem::isPlatformWayland()) {
            KWindowSystem::setCurrentXdgActivationToken(startupId);
        }
    }

    KWindowSystem::activateWindow(m_window);
#endif
}

#include "moc_windowcontroller.cpp"
