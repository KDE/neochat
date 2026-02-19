// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QList>
#include <QTextDocumentFragment>

#include "enums/messagecomponenttype.h"
#include "messagecomponent.h"

namespace Block
{
struct CacheItem {
    MessageComponentType::Type type = MessageComponentType::Other;
    QTextDocumentFragment content;
};

class Cache : private QList<CacheItem>
{
public:
    using QList<CacheItem>::constBegin, QList<CacheItem>::constEnd;
    using QList<CacheItem>::isEmpty;
    using QList<CacheItem>::clear;

    void fill(QList<MessageComponent> components);
};
}
