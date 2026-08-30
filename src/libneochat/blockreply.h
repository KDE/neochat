// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "block.h"
#include "models/messagecontentmodel.h"

namespace Blocks
{
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
    ReplyCacheItem(MessageContentModel *blockModel);
    ~ReplyCacheItem();

    MessageContentModel *blockModel;
};

/**
 * @class ReplyBlock
 *
 * A block to visualize a reply.
 */
class ReplyBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The block model for the message being replied to.
     */
    Q_PROPERTY(MessageContentModel *blockModel READ blockModel WRITE setBlockModel NOTIFY blockModelChanged)

public:
    ReplyBlock(MessageContentModel *blockModel, QObject *parent);
    ReplyBlock(ReplyCacheItem *item, QObject *parent);
    ~ReplyBlock() override;

    [[nodiscard]] MessageContentModel *blockModel() const;
    void setBlockModel(MessageContentModel *blockModel);

    [[nodiscard]] CacheItemPtr toCacheItem() override;

Q_SIGNALS:
    void blockModelChanged();

private:
    QPointer<MessageContentModel> m_blockModel;
    bool m_cacheCreated = false;
};
}
