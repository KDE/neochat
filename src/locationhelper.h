// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later
#pragma once

#include <QMetaType>
#include <QObject>
#include <QQmlEngine>
#include <QRectF>

/** Location related helper functions for QML. */
class LocationHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
public:
    /** Unite two rectanlges. */
    Q_INVOKABLE static QRectF unite(const QRectF &r1, const QRectF &r2);
    /** Returns the center of @p r. */
    Q_INVOKABLE static QPointF center(const QRectF &r);

    /** Returns the highest zoom level to fit @r into a map of size @p mapWidth x @p mapHeight. */
    Q_INVOKABLE static float zoomToFit(const QRectF &r, float mapWidth, float mapHeight);
};

Q_DECLARE_METATYPE(LocationHelper)
