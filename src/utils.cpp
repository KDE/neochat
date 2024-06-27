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

#include "moc_utils.cpp"
