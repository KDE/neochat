// SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <KStatusNotifierItem>

class TrayIcon : public KStatusNotifierItem
{
    Q_OBJECT
public:
    TrayIcon(QObject *parent = nullptr);

    void show();
    void hide();

Q_SIGNALS:
    void showWindow();
};
