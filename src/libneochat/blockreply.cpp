// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "blockreply.h"

using namespace Blocks;

ReplyCacheItem::ReplyCacheItem(MessageContentModel *blockModel)
    : CacheItem(Reply)
    , blockModel(blockModel)
{
}

ReplyCacheItem::~ReplyCacheItem()
{
    // If the model has a parent it's currently owned by a ReplyBlock so only delete
    // if parent is nullptr.
    if (blockModel && !blockModel->parent()) {
        blockModel->deleteLater();
    }
}

ReplyBlock::ReplyBlock(MessageContentModel *blockModel, QObject *parent)
    : Block(Reply, parent)
    , m_blockModel(blockModel)
{
    Q_ASSERT(m_blockModel);
    m_blockModel->setParent(this);
}

ReplyBlock::ReplyBlock(ReplyCacheItem *item, QObject *parent)
    : Block(item, parent)
    , m_blockModel(item->blockModel)
{
    Q_ASSERT(m_blockModel);
    m_blockModel->setParent(this);
}

ReplyBlock::~ReplyBlock()
{
    m_blockModel->setParent(nullptr);
    // If no cache has been created from this then we need to clean up.
    if (!m_cacheCreated) {
        m_blockModel->deleteLater();
    }
}

MessageContentModel *ReplyBlock::blockModel() const
{
    return m_blockModel;
}

void ReplyBlock::setBlockModel(MessageContentModel *blockModel)
{
    if (blockModel == m_blockModel) {
        return;
    }
    m_blockModel = blockModel;
    Q_EMIT blockModelChanged();
}

CacheItemPtr ReplyBlock::toCacheItem()
{
    m_cacheCreated = true;
    return std::make_unique<ReplyCacheItem>(m_blockModel);
}

#include "moc_blockreply.cpp"
