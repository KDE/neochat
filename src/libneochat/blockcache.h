// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QList>
#include <QTextDocumentFragment>

#include "block.h"
#include "enums/blocktype.h"

namespace Blocks
{
/**
 * @class CacheItem
 *
 * A structure to define an item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class CacheItem
{
public:
    CacheItem(Type type);
    virtual ~CacheItem();

    Type type = Other;

    /**
     * @brief Return the contents of the CacheItem as a single string.
     */
    virtual QString toString() const;

    static bool richTextActive;
};

/**
 * @class TextCacheItem
 *
 * A structure to define a text item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class TextCacheItem : public CacheItem
{
public:
    TextCacheItem(Type type, const QTextDocumentFragment &content);

    QTextDocumentFragment content;

    QString toString() const override;
};

/**
 * @class FileCacheItem
 *
 * A structure to define a file item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class FileCacheItem : public CacheItem
{
public:
    FileCacheItem(Type type, const QUrl &source);

    QUrl source;
};

/**
 * @class ReplyCacheItem
 *
 * A structure to define a reply item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class ReplyCacheItem : public CacheItem
{
public:
    ReplyCacheItem(Type type, const QString &id);

    QString id;
};

/**
 * @class Cache
 *
 * A class to cache the contents of a ChatBarMessageContentModel.
 *
 * We can't store the actual content items as the QTextDocuments are attached to
 * text items that may be deleted by the QML engine. Instead we get the contents
 * as a QTextDocumentFragment in a Blocks::CacheItem which can be used to reconstruct the
 * model later.
 *
 * @sa ChatBarMessageContentModel, QTextDocumentFragment, QTextDocument, Blocks::CacheItem
 */
class Cache
{
    using CacheItems = std::vector<std::unique_ptr<CacheItem>>;

public:
    CacheItems::iterator begin();
    CacheItems::iterator end();
    CacheItems::const_iterator begin() const;
    CacheItems::const_iterator end() const;
    CacheItems::const_iterator cbegin() const;
    CacheItems::const_iterator cend() const;

    /**
     * @brief Whether the Cache has any CacheItem in it.
     *
     * @sa CacheItem
     */
    bool empty() const;

    /**
     * @brief Return the CacheItem at the given index.
     *
     * nullptr if i is not a valid index.
     *
     * @sa CacheItem
     */
    const CacheItem *at(qsizetype i) const;

    /**
     * @brief Prepend the given CacheItem to the cache.
     *
     * @sa CacheItem
     */
    void prepend(std::unique_ptr<CacheItem> item);

    /**
     * @brief Append the given CacheItem to the cache.
     *
     * @sa CacheItem
     */
    void append(std::unique_ptr<CacheItem> item);

    /**
     * @brief Fill the cache from a list of Blocks.
     *
     * @sa Block
     */
    void fill(const BlockPtrs &components);

    /**
     * @brief Remove the CacheItem at the given index from the cache.
     *
     * @sa CacheItem
     */
    void removeAt(qsizetype i);

    /**
     * @brief Clear the Cache.
     */
    void clear();

    /**
     * @brief Return the contents of the Cache as a single string.
     */
    QString toString() const;

private:
    CacheItems m_items;
};
}
