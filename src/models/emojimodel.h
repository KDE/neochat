// SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <KConfigGroup>
#include <KSharedConfig>
#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>

struct Emoji {
    Emoji(QString unicode, QString shortname, bool isCustom = false)
        : unicode(std::move(unicode))
        , shortName(std::move(shortname))
        , isCustom(isCustom)
    {
    }
    Emoji(QString unicode, QString shortname, QString description)
        : unicode(std::move(unicode))
        , shortName(std::move(shortname))
        , description(std::move(description))
    {
    }
    Emoji() = default;

    QString unicode;
    QString shortName;
    QString description;
    bool isCustom = false;

    Q_GADGET
    Q_PROPERTY(QString unicode MEMBER unicode)
    Q_PROPERTY(QString shortName MEMBER shortName)
    Q_PROPERTY(QString description MEMBER description)
    Q_PROPERTY(bool isCustom MEMBER isCustom)
};

Q_DECLARE_METATYPE(Emoji)

/**
 * @class EmojiModel
 *
 * This class defines the model for visualising a list of emojis.
 */
class EmojiModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /**
     * @brief Return a list of emoji categories.
     *
     * @note No custom emoji categories will be included.
     */
    Q_PROPERTY(QVariantList categories READ categories CONSTANT)

    /**
     * @brief Return a list of emoji categories with custom emojis.
     */
    Q_PROPERTY(QVariantList categoriesWithCustom READ categoriesWithCustom CONSTANT)

public:
    static EmojiModel &instance()
    {
        static EmojiModel _instance;
        return _instance;
    }
    static EmojiModel *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    /**
     * @brief Defines the model roles.
     */
    enum RoleNames {
        ShortNameRole = Qt::DisplayRole, /**< The name of the emoji. */
        UnicodeRole, /**< The unicode character of the emoji. */
        InvalidRole = 50, /**< Invalid, reserved. */
        DisplayRole = 51, /**< The display text for an emoji. */
        ReplacedTextRole = 52, /**< The text to replace the short name with (i.e. the unicode character). */
        DescriptionRole = 53, /**< The long description of an emoji. */
    };
    Q_ENUM(RoleNames)

    /**
     * @brief Defines the potential categories an emoji can be placed in.
     */
    enum Category {
        Custom, /**< A custom user emoji. */
        Search, /**< The results of a filter. */
        SearchNoCustom, /**< The results of a filter with no custom emojis. */
        History, /**< Recently used emojis. */
        HistoryNoCustom, /**< Recently used emojis with no custom emojis. */
        Smileys, /**< Smileys & emotion emojis. */
        People, /**< People & Body emojis. */
        Nature, /**< Animals & Nature emojis. */
        Food, /**< Food & Drink emojis. */
        Activities, /**< Activities emojis. */
        Travel, /**< Travel & Places emojis. */
        Objects, /**< Objects emojis. */
        Symbols, /**< Symbols emojis. */
        Flags, /**< Flags emojis. */
        Component, /**< ??? */
    };
    Q_ENUM(Category)

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa RoleNames, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Return a filtered list of emojis.
     *
     * @note This includes custom emojis, use filterModelNoCustom to return a result
     *       without custom emojis.
     *
     * @sa filterModelNoCustom
     */
    Q_INVOKABLE static QVariantList filterModel(const QString &filter, bool limit = true);

    /**
     * @brief Return a filtered list of emojis without custom emojis.
     *
     * @note Use filterModel to return a result with custom emojis.
     *
     * @sa filterModel
     */
    Q_INVOKABLE static QVariantList filterModelNoCustom(const QString &filter, bool limit = true);

    /**
     * @brief Return a list of emojis for the given category.
     */
    Q_INVOKABLE QVariantList emojis(Category category) const;

    /**
     * @brief Return a list of emoji tones for the given base emoji.
     */
    Q_INVOKABLE QVariantList tones(const QString &baseEmoji) const;

    /**
     * @brief Return a list of the last used emoji shortnames
     */
    QStringList lastUsedEmojis() const;

    QVariantList categories() const;
    QVariantList categoriesWithCustom() const;

Q_SIGNALS:
    void historyChanged();

public Q_SLOTS:
    void emojiUsed(const QVariant &modelData);

private:
    static QHash<Category, QVariantList> _emojis;

    /// Returns QVariants containing the last used Emojis
    QVariantList emojiHistory() const;

    KSharedConfig::Ptr m_config;
    KConfigGroup m_configGroup;
    EmojiModel(QObject *parent = nullptr);
};
