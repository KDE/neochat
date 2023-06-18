// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "locationhelper.h"

#include <cmath>

QRectF LocationHelper::unite(const QRectF &r1, const QRectF &r2)
{
    // this looks weird but is actually intentional as we need to handle point-like "rects" as well
    if ((!r1.isEmpty() || r1.isNull()) && (!r2.isEmpty() || r2.isNull())) {
        return r1 | r2;
    }
    return (!r1.isEmpty() || r1.isNull()) ? r1 : r2;
}

QPointF LocationHelper::center(const QRectF &r)
{
    return r.center();
}

constexpr inline double degToRad(double deg)
{
    return deg / 180.0 * M_PI;
}

static QPointF mercatorProject(double lat, double lon, double zoom)
{
    const auto x = (256.0 / (2.0 * M_PI)) * std::pow(2.0, zoom) * (degToRad(lon) + M_PI);
    const auto y = (256.0 / (2.0 * M_PI)) * std::pow(2.0, zoom) * (M_PI - std::log(std::tan(M_PI / 4.0 + degToRad(lat) / 2.0)));
    return QPointF(x, y);
}

float LocationHelper::zoomToFit(const QRectF &r, float mapWidth, float mapHeight)
{
    const auto p1 = mercatorProject(r.bottomLeft().y(), r.bottomLeft().x(), 1.0);
    const auto p2 = mercatorProject(r.topRight().y(), r.topRight().x(), 1.0);

    const auto zx = std::log2((mapWidth / (p2.x() - p1.x())));
    const auto zy = std::log2((mapHeight / (p2.y() - p1.y())));
    const auto z = std::min(zx, zy);

    return std::clamp(z, 5.0, 18.0);
}
