// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "utils.h"

#include <QJsonDocument>

bool QmlUtils::isValidJson(const QByteArray &json)
{
    return !QJsonDocument::fromJson(json).isNull();
}
