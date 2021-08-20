// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QQuickImageProvider>

class BlurhashImageProvider : public QQuickImageProvider
{
public:
    BlurhashImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};