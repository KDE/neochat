// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QQmlApplicationEngine>
#include <QTest>
#include <QWindow>

#include <KConfig>
#include <KSharedConfig>
#include <KWindowConfig>

#include "windowcontroller.h"

class WindowControllerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void nullWindow();
    void geometry();
    void showAndRaise();
    void toggle();

    void cleanup();
};

// Basically don't crash when no window is set.
void WindowControllerTest::nullWindow()
{
    auto &instance = WindowController::instance();
    QCOMPARE(instance.window(), nullptr);

    instance.restoreGeometry();
    instance.saveGeometry();
    instance.showAndRaiseWindow({});
    instance.toggleWindow();
}

void WindowControllerTest::geometry()
{
    auto &instance = WindowController::instance();

    QWindow window;
    window.setGeometry(0, 0, 200, 200);
    instance.setWindow(&window);
    QCOMPARE(instance.window(), &window);

    instance.saveGeometry();
    const auto stateConfig = KSharedConfig::openStateConfig();
    KConfigGroup windowGroup = stateConfig->group(QStringLiteral("Window"));
    QCOMPARE(KWindowConfig::hasSavedWindowSize(windowGroup), true);

    window.setGeometry(0, 0, 400, 400);
    QCOMPARE(window.geometry(), QRect(0, 0, 400, 400));
    instance.restoreGeometry();
    QCOMPARE(window.geometry(), QRect(0, 0, 200, 200));
}

void WindowControllerTest::showAndRaise()
{
    auto &instance = WindowController::instance();
    QWindow window;
    instance.setWindow(&window);
    QCOMPARE(window.isVisible(), false);

    instance.showAndRaiseWindow({});
    QCOMPARE(window.isVisible(), true);
}

void WindowControllerTest::cleanup()
{
    auto &instance = WindowController::instance();
    instance.setWindow(nullptr);
    QCOMPARE(instance.window(), nullptr);
}

void WindowControllerTest::toggle()
{
    auto &instance = WindowController::instance();
    QWindow window;
    instance.setWindow(&window);
    QCOMPARE(window.isVisible(), false);
    instance.toggleWindow();
    QCOMPARE(window.isVisible(), true);
    instance.toggleWindow();
    QCOMPARE(window.isVisible(), false);

    // A window is classed as visible by qt when minimized but to the user this is not visible.
    // So in this case we expect to show it even though visibility is technically true.
    window.setVisibility(QWindow::Minimized);
    QCOMPARE(window.windowState(), Qt::WindowMinimized);
    QCOMPARE(window.isVisible(), true);
    instance.toggleWindow();
    QCOMPARE(window.windowState(), Qt::WindowNoState);
    QCOMPARE(window.isVisible(), true);
    instance.toggleWindow();
    QCOMPARE(window.windowState(), Qt::WindowNoState);
    QCOMPARE(window.isVisible(), false);
}

QTEST_MAIN(WindowControllerTest)
#include "windowcontrollertest.moc"
