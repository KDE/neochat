// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QList>
#include <QTextDocumentFragment>

#include "enums/messagecomponenttype.h"
#include "messagecomponent.h"

namespace Block
{
/**
 * @struct CacheItem
 *
 * A structure to define an item stored in a Block::Cache.
 *
 * @sa Block::Cache
 */
struct CacheItem {
    MessageComponentType::Type type = MessageComponentType::Other;
    QTextDocumentFragment content;

    /**
     * @brief Return the contents of the CacheItem as a single string.
     */
    QString toString() const;
};

/**
 * @class Cache
 *
 * A class to cache the contents of a ChatBarMessageContentModel.
 *
 * We can't store the actual content items as the QTextDocuments are attached to
 * text items that may be deleted by the QML engine. Instead we get the contents
 * as a QTextDocumentFragment in a Block::CacheItem which can be used to reconstruct the
 * model later.
 *
 * @sa ChatBarMessageContentModel, QTextDocumentFragment, QTextDocument, Block::CacheItem
 */
class Cache : private QList<CacheItem>
{
public:
    using QList<CacheItem>::constBegin, QList<CacheItem>::constEnd;
    using QList<CacheItem>::isEmpty;
    using QList<CacheItem>::clear;
    using QList<CacheItem>::append, QList<CacheItem>::operator+=, QList<CacheItem>::operator<<;

    /**
     * @brief Fill the cache from a list of MessageComponents.
     *
     * @sa MessageComponent
     */
    void fill(QList<MessageComponent> components);

    /**
     * @brief Return the contents of the Cache as a single string.
     */
    QString toString() const;
};
}
