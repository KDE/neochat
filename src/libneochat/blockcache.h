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
 * @class CacheItem
 *
 * A structure to define an item stored in a Block::Cache.
 *
 * @sa Block::Cache
 */
class CacheItem
{
public:
    CacheItem(MessageComponentType::Type type);
    virtual ~CacheItem();

    MessageComponentType::Type type = MessageComponentType::Other;

    /**
     * @brief Return the contents of the CacheItem as a single string.
     */
    virtual QString toString() const;

    static bool richTextActive;
};

/**
 * @class TextCacheItem
 *
 * A structure to define a text item stored in a Block::Cache.
 *
 * @sa Block::Cache
 */
class TextCacheItem : public CacheItem
{
public:
    TextCacheItem(MessageComponentType::Type type, const QTextDocumentFragment &content);

    QTextDocumentFragment content;

    QString toString() const override;
};

/**
 * @class FileCacheItem
 *
 * A structure to define a file item stored in a Block::Cache.
 *
 * @sa Block::Cache
 */
class FileCacheItem : public CacheItem
{
public:
    FileCacheItem(MessageComponentType::Type type, const QUrl &source);

    QUrl source;
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
     * @brief Fill the cache from a list of MessageComponents.
     *
     * @sa MessageComponent
     */
    void fill(QList<MessageComponent> components);

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
