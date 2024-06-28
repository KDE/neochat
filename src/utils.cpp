// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "utils.h"

#include <Quotient/connection.h>

#include <QJsonDocument>

using namespace Quotient;

bool QmlUtils::isValidJson(const QByteArray &json)
{
    return !QJsonDocument::fromJson(json).isNull();
}

QString QmlUtils::escapeString(const QString &string)
{
    return string.toHtmlEscaped();
}

QColor QmlUtils::getUserColor(qreal hueF)
{
    const auto lightness = static_cast<QGuiApplication *>(QGuiApplication::instance())->palette().color(QPalette::Active, QPalette::Window).lightnessF();
    // https://github.com/quotient-im/libQuotient/wiki/User-color-coding-standard-draft-proposal
    return QColor::fromHslF(hueF, 1, -0.7 * lightness + 0.9, 1);
}

#include "moc_utils.cpp"
