// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "events/imagepackevent.h"

#include <QAbstractListModel>
#include <QCoroTask>
#include <QObject>
#include <QPointer>
#include <QVector>

#include <Quotient/connection.h>

/**
 * @class AccountEmoticonModel
 *
 * This class defines the model for visualising the account stickers and emojis.
 *
 * This is based upon the im.ponies.user_emotes spec (MSC2545).
 */
class AccountEmoticonModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * @brief The connection to get emoticons from.
     */
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

public:
    enum Roles {
        UrlRole = Qt::UserRole + 1, /**< The URL for the emoticon. */
        ShortCodeRole, /**< The shortcode for the emoticon. */
        BodyRole, //**< A textual description of the emoticon */
        IsStickerRole, //**< Whether this emoticon is a sticker */
        IsEmojiRole, //**< Whether this emoticon is an emoji */
    };

    explicit AccountEmoticonModel(QObject *parent = nullptr);

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
     * @brief Deletes the emoticon at the given index.
     */
    Q_INVOKABLE void deleteEmoticon(int index);

    /**
     * @brief Changes the description for the emoticon at the given index.
     */
    Q_INVOKABLE void setEmoticonBody(int index, const QString &text);

    /**
     * @brief Changes the shortcode for the emoticon at the given index.
     */
    Q_INVOKABLE void setEmoticonShortcode(int index, const QString &shortCode);

    /**
     * @brief Changes the image for the emoticon at the given index.
     */
    Q_INVOKABLE void setEmoticonImage(int index, const QUrl &source);

    /**
     * @brief Add an emoticon with the given parameters.
     */
    Q_INVOKABLE void addEmoticon(const QUrl &source, const QString &shortcode, const QString &description, const QString &type);

Q_SIGNALS:
    void connectionChanged();

private:
    std::optional<Quotient::ImagePackEventContent> m_images;
    QPointer<Quotient::Connection> m_connection;
    QCoro::Task<void> doSetEmoticonImage(int index, QUrl source);
    QCoro::Task<void> doAddEmoticon(QUrl source, QString shortcode, QString description, QString type);

    void reloadEmoticons();
};
