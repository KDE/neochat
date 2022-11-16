// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>

class UrlHelper : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void openUrl(const QUrl &url);
    Q_INVOKABLE void copyTo(const QUrl &origin, const QUrl &destination);
};
