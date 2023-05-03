// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QUrl>

/**
 * @class UrlHelper
 *
 * A class to help manage URLs.
 */
class UrlHelper : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Open the given URL in an appropriate app.
     */
    Q_INVOKABLE void openUrl(const QUrl &url);

    /**
     * @brief Copy the given URL to the given location.
     */
    Q_INVOKABLE void copyTo(const QUrl &origin, const QUrl &destination);
};
