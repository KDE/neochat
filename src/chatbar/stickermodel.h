// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QPointer>
#include <qqmlintegration.h>

#include "stickerlist.h"

/**
 * @class StickerModel
 *
 * A model to visualise a list of stickers.
 *
 * The stickers are passed in as a QList of StickerLists.
 */
class StickerModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The StickerLists to get stickers from.
     *
     * @sa StickerList
     */
    Q_PROPERTY(QList<StickerList *> stickerLists READ stickerLists WRITE setStickerLists NOTIFY stickerListsChanged)

    /**
     * @brief The index of the StickerLists to show.
     *
     * @sa StickerList
     */
    Q_PROPERTY(int listIndex READ listIndex WRITE setListIndex NOTIFY listIndexChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        SourceRole = Qt::UserRole + 1, /**< The source mxc URL for the image. */
        NameRole, /**< The image name, will be short code followed by the description if available. */
        ShortCodeRole, /**< The image short code. */
        DescriptionRole, /**< The image description if available. */
        PixelSizeRole, /**< The image with and high in pixels for the sticker. */
        SizeRole, /**< The image size in bytes for the sticker. */
        MimeRole, /**< The image mime type for the sticker. */
    };

    explicit StickerModel(QObject *parent = nullptr);

    [[nodiscard]] QList<StickerList *> stickerLists() const;
    void setStickerLists(const QList<StickerList *> &stickerLists);

    [[nodiscard]] int listIndex() const;
    void setListIndex(int listIndex);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void stickerListsChanged();
    void listIndexChanged();

private:
    QList<QPointer<StickerList>> m_lists;
    void listCompletionsAdded(StickerList *list, qsizetype first, qsizetype last);
    void listCompletionsRemoved(StickerList *list, qsizetype first, qsizetype last);

    int m_listIndex = 0;
};
