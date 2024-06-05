// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imagecontentfiltermodel.h"

#include "imagecontentmanager.h"

ImageContentFilterModel::ImageContentFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    updateSourceModel();
}

bool ImageContentFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    auto index = sourceModel()->index(sourceRow, 0);
    return ((index.data(ImageContentRole::IsEmojiRole).toBool() && emojis()) || (index.data(ImageContentRole::IsStickerRole).toBool() && stickers()))
        && sourceModel()->index(sourceRow, 0).data(ImageContentRole::DisplayNameRole).toString().contains(m_searchText, Qt::CaseInsensitive);
}

bool ImageContentFilterModel::stickers() const
{
    return m_stickers;
}

void ImageContentFilterModel::setStickers(bool stickers)
{
    m_stickers = stickers;
    Q_EMIT stickersChanged();
    invalidateFilter();
}

bool ImageContentFilterModel::emojis() const
{
    return m_emojis;
}

void ImageContentFilterModel::setEmojis(bool emojis)
{
    m_emojis = emojis;
    Q_EMIT emojisChanged();
    invalidateFilter();
}

void ImageContentFilterModel::setCategory(const QString &category)
{
    if (category == m_category) {
        return;
    }
    m_category = category;
    Q_EMIT categoryChanged();
    updateSourceModel();
}

QString ImageContentFilterModel::category() const
{
    return m_category;
}

void ImageContentFilterModel::setSearchText(const QString &searchText)
{
    if (searchText == m_searchText) {
        return;
    }
    m_searchText = searchText;
    Q_EMIT searchTextChanged();
    invalidateFilter();
    updateSourceModel();
}

QString ImageContentFilterModel::searchText() const
{
    return m_searchText;
}

void ImageContentFilterModel::updateSourceModel()
{
    if (!m_searchText.isEmpty()) {
        if (sourceModel() != &m_allImageContentModel) {
            setSourceModel(&m_allImageContentModel);
        }
    } else if (m_category == QStringLiteral("history")) {
        setSourceModel(&m_recentImageContentProxyModel);
    } else {
        if (sourceModel() != &m_imageContentModel) {
            setSourceModel(&m_imageContentModel);
        }
        m_imageContentModel.setCategory(m_category);
    }
}
