// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QSortFilterProxyModel>
#include <QQmlEngine>

#include "allimagecontentmodel.h"
#include "imagecontentmodel.h"
#include "recentimagecontentproxymodel.h"

class ImageContentFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool stickers READ stickers WRITE setStickers NOTIFY stickersChanged)
    Q_PROPERTY(bool emojis READ emojis WRITE setEmojis NOTIFY emojisChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    explicit ImageContentFilterModel(QObject *parent = nullptr);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    [[nodiscard]] bool stickers() const;
    void setStickers(bool stickers);

    [[nodiscard]] bool emojis() const;
    void setEmojis(bool emojis);

    QString category() const;
    void setCategory(const QString &category);

    QString searchText() const;
    void setSearchText(const QString &text);

Q_SIGNALS:
    void stickersChanged();
    void emojisChanged();
    void categoryChanged();
    void searchTextChanged();

private:
    bool m_stickers = true;
    bool m_emojis = true;
    QString m_category;
    QString m_searchText;

    AllImageContentModel m_allImageContentModel;
    RecentImageContentProxyModel m_recentImageContentProxyModel;
    ImageContentModel m_imageContentModel;

    void updateSourceModel();
};
