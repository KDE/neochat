// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QRegularExpression>
#include <memory>

struct CustomEmoji {
    QString name; // with :semicolons:
    QString url; // mxc://
    QRegularExpression regexp;

    Q_GADGET
    Q_PROPERTY(QString unicode MEMBER url)
    Q_PROPERTY(QString name MEMBER name)
};

/**
 * @class CustomEmojiModel
 *
 * This class defines the model for custom user emojis.
 *
 * This is based upon the im.ponies.user_emotes spec (MSC2545).
 */
class CustomEmojiModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        Name = Qt::DisplayRole, /**< The name of the emoji. */
        ImageURL, /**< The URL for the custom emoji. */
        ModelData, /**< for emulating the regular emoji model's usage, otherwise the UI code would get too complicated. */
        MxcUrl = 50, /**< The mxc source URL for the custom emoji. */
        DisplayRole = 51, /**< The name of the emoji. For compatibility with EmojiModel. */
        ReplacedTextRole = 52, /**< The name of the emoji. For compatibility with EmojiModel. */
        DescriptionRole = 53, /**< Invalid, reserved. For compatibility with EmojiModel. */
    };
    Q_ENUM(Roles)

    static CustomEmojiModel &instance()
    {
        static CustomEmojiModel _instance;
        return _instance;
    }

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Substitute any custom emojis for an image in the input text.
     */
    Q_INVOKABLE QString preprocessText(const QString &it);

    /**
     * @brief Return a list of custom emojis where the name contains the filter text.
     */
    Q_INVOKABLE QVariantList filterModel(const QString &filter);

    /**
     * @brief Add a new emoji to the model.
     */
    Q_INVOKABLE void addEmoji(const QString &name, const QUrl &location);

    /**
     * @brief Remove an emoji from the model.
     */
    Q_INVOKABLE void removeEmoji(const QString &name);

private:
    explicit CustomEmojiModel(QObject *parent = nullptr);
    QList<CustomEmoji> m_emojis;

    void fetchEmojis();
};
