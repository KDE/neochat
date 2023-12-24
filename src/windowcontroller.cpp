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

#include <KSharedConfig>
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
    const auto stateConfig = KSharedConfig::openStateConfig();
    const KConfigGroup windowGroup = stateConfig->group(QStringLiteral("Window"));

    KWindowConfig::restoreWindowSize(m_window, windowGroup);
    KWindowConfig::restoreWindowPosition(m_window, windowGroup);
}

void WindowController::saveGeometry()
{
    const auto stateConfig = KSharedConfig::openStateConfig();
    KConfigGroup windowGroup = stateConfig->group(QStringLiteral("Window"));

    KWindowConfig::saveWindowPosition(m_window, windowGroup);
    KWindowConfig::saveWindowSize(m_window, windowGroup);

    stateConfig->sync();
}

void WindowController::showAndRaiseWindow(const QString &startupId)
{
    if (m_window == nullptr) {
        return;
    }
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

bool WindowController::hasWindowSystem() const
{
#ifdef HAVE_WINDOWSYSTEM
    return true;
#else
    return false;
#endif
}

void WindowController::setBlur(QQuickItem *item, bool blur)
{
#ifdef HAVE_WINDOWSYSTEM
    auto setWindows = [item, blur]() {
        auto reg = QRect(QPoint(0, 0), item->window()->size());
        KWindowEffects::enableBackgroundContrast(item->window(), blur, 1, 1, 1, reg);
        KWindowEffects::enableBlurBehind(item->window(), blur, reg);
    };

    disconnect(item->window(), &QQuickWindow::heightChanged, this, nullptr);
    disconnect(item->window(), &QQuickWindow::widthChanged, this, nullptr);
    connect(item->window(), &QQuickWindow::heightChanged, this, setWindows);
    connect(item->window(), &QQuickWindow::widthChanged, this, setWindows);
    setWindows();
#endif
}

void WindowController::toggleWindow()
{
    if (m_window == nullptr) {
        return;
    }
    if (window()->isVisible()) {
        if (window()->windowStates() & Qt::WindowMinimized) {
            window()->showNormal();
            window()->requestActivate();
        } else {
            window()->close();
        }
    } else {
        showAndRaiseWindow({});
        window()->requestActivate();
    }
}

#include "moc_windowcontroller.cpp"
