// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QWindow>

class WindowController : public QObject
{
    Q_OBJECT

public:
    static WindowController &instance();

    void setWindow(QWindow *window);

    void restoreGeometry();
    void saveGeometry();
    void showAndRaiseWindow(const QString &xdgActivationToken);

private:
    WindowController() = default;

    QWindow *m_window = nullptr;
};
