// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "events/imagepackevent.h"
#include <QAbstractListModel>
#include <QPointer>
#include <QVector>

class NeoChatRoom;

/**
 * @class ImagePacksModel
 *
 * Defines the model for visualising image packs.
 *
 * See Matrix MSC2545 for more details on image packs.
 * https://github.com/Sorunome/matrix-doc/blob/soru/emotes/proposals/2545-emotes.md
 */
class ImagePacksModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The current room that the model is being used in.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief Whether sticker image packs should be shown.
     */
    Q_PROPERTY(bool showStickers READ showStickers WRITE setShowStickers NOTIFY showStickersChanged)

    /**
     * @brief Whether emoticon image packs should be shown.
     */
    Q_PROPERTY(bool showEmoticons READ showEmoticons WRITE setShowEmoticons NOTIFY showEmoticonsChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        DisplayNameRole = Qt::DisplayRole, /**< The display name of the image pack. */
        AvatarUrlRole, /**< The source mxc URL for the pack avatar. */
        AttributionRole, /**< The attribution for the pack author(s). */
        IdRole, /**< The ID of the image pack. */
    };
    Q_ENUM(Roles);

    explicit ImagePacksModel(QObject *parent = nullptr);

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

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    [[nodiscard]] bool showStickers() const;
    void setShowStickers(bool showStickers);

    [[nodiscard]] bool showEmoticons() const;
    void setShowEmoticons(bool showEmoticons);

    /**
     * @brief Return a vector of the images in the pack at the given index.
     */
    [[nodiscard]] QVector<Quotient::ImagePackEventContent::ImagePackImage> images(int index);

Q_SIGNALS:
    void roomChanged();
    void showStickersChanged();
    void showEmoticonsChanged();
    void imagesLoaded();

private:
    QPointer<NeoChatRoom> m_room;
    QVector<Quotient::ImagePackEventContent> m_events;
    bool m_showStickers = true;
    bool m_showEmoticons = true;
    void reloadImages();
};
