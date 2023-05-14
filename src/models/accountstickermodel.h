// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "events/imagepackevent.h"
#include <QAbstractListModel>
#include <QCoroTask>
#include <QObject>
#include <QPointer>
#include <QVector>
#include <connection.h>

class ImagePacksModel;

/**
 * @class AccountStickerModel
 *
 * This class defines the model for visualising the account stickers.
 *
 * This is based upon the im.ponies.user_emotes spec (MSC2545).
 */
class AccountStickerModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * @brief The connection to get stickers from.
     */
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

public:
    enum Roles {
        UrlRole = Qt::UserRole + 1, /**< The URL for the sticker. */
        ShortCodeRole, /**< The shortcode for the sticker. */
        BodyRole, //**< A textual description of the sticker */
        IsStickerRole, //**< Whether this emoticon is a sticker */
        IsEmojiRole, //**< Whether this emoticon is an emoji */
    };

    explicit AccountStickerModel(QObject *parent = nullptr);

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &index) const override;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *connection);

    /**
     * @brief Deletes the sticker at the given index.
     */
    Q_INVOKABLE void deleteSticker(int index);

    /**
     * @brief Changes the description for the sticker at the given index.
     */
    Q_INVOKABLE void setStickerBody(int index, const QString &text);

    /**
     * @brief Changes the shortcode for the sticker at the given index.
     */
    Q_INVOKABLE void setStickerShortcode(int index, const QString &shortCode);

    /**
     * @brief Changes the image for the sticker at the given index.
     */
    Q_INVOKABLE void setStickerImage(int index, const QUrl &source);

    /**
     * @brief Adds a sticker with the given parameters.
     */
    Q_INVOKABLE void addSticker(const QUrl &source, const QString &shortcode, const QString &description);

Q_SIGNALS:
    void connectionChanged();

private:
    std::optional<Quotient::ImagePackEventContent> m_images;
    QPointer<Quotient::Connection> m_connection;
    QCoro::Task<void> doSetStickerImage(int index, QUrl source);
    QCoro::Task<void> doAddSticker(QUrl source, QString shortcode, QString description);

    void reloadStickers();
};
