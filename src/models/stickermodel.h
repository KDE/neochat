// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "events/imagepackevent.h"
#include "neochatroom.h"
#include <QAbstractListModel>
#include <QObject>
#include <QVector>

class ImagePacksModel;

/**
 * @class StickerModel
 *
 * A model to visualise a set of stickers.
 *
 * The stickers are obtained from a Matrix image pack. See Matrix MSC2545 for more details.
 * https://github.com/Sorunome/matrix-doc/blob/soru/emotes/proposals/2545-emotes.md
 */
class StickerModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The image pack that the stickers come from.
     *
     * @sa ImagePacksModel
     */
    Q_PROPERTY(ImagePacksModel *model READ model WRITE setModel NOTIFY modelChanged)

    /**
     * @brief The index of the pack in the ImagePacksModel.
     *
     * @sa ImagePacksModel
     */
    Q_PROPERTY(int packIndex READ packIndex WRITE setPackIndex NOTIFY packIndexChanged)

    /**
     * @brief The current room that the model is being used in.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        Url = Qt::UserRole + 1, /**< The source mxc URL for the image. */
        Body, /**< The image caption, if any. */
    };

    explicit StickerModel(QObject *parent = nullptr);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &index) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] ImagePacksModel *model() const;
    void setModel(ImagePacksModel *model);

    [[nodiscard]] int packIndex() const;
    void setPackIndex(int index);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Post the sticker at the given index as an event in the room.
     */
    Q_INVOKABLE void postSticker(int index);

Q_SIGNALS:
    void roomChanged();
    void modelChanged();
    void packIndexChanged();

private:
    ImagePacksModel *m_model = nullptr;
    int m_index = 0;
    QVector<Quotient::ImagePackEventContent::ImagePackImage> m_images;
    NeoChatRoom *m_room;
};
