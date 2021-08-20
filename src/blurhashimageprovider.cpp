// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "blurhashimageprovider.h"

#include <QImage>
#include <QString>

#include "blurhash.h"

BlurhashImageProvider::BlurhashImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage BlurhashImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    if (id.isEmpty()) {
        return QImage();
    }
    *size = requestedSize;
    if (size->width() == -1) {
        size->setWidth(256);
    }
    if (size->height() == -1) {
        size->setHeight(256);
    }
    auto data = decode(QUrl::fromPercentEncoding(id.toLatin1()).toLatin1().data(), size->width(), size->height(), 1, 3);
    QImage image(data, size->width(), size->height(), size->width() * 3, QImage::Format_RGB888, free, data);
    return image;
}