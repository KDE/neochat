// SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QSettings>

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

    friend QDataStream &operator<<(QDataStream &arch, const Emoji &object)
    {
        arch << object.unicode;
        arch << object.shortName;
        return arch;
    }

    friend QDataStream &operator>>(QDataStream &arch, Emoji &object)
    {
        arch >> object.unicode;
        arch >> object.shortName;
        object.isCustom = object.unicode.startsWith("image://");
        return arch;
    }

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

class EmojiModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)
    Q_PROPERTY(QVariantList categories READ categories CONSTANT)
    Q_PROPERTY(QVariantList categoriesWithCustom READ categoriesWithCustom CONSTANT)

public:
    static EmojiModel &instance()
    {
        static EmojiModel _instance;
        return _instance;
    }

    enum RoleNames {
        ShortNameRole = Qt::DisplayRole,
        UnicodeRole,
        InvalidRole = 50,
        DisplayRole = 51,
        ReplacedTextRole = 52,
        DescriptionRole = 53,
    };
    Q_ENUM(RoleNames);

    enum Category {
        Custom,
        Search,
        SearchNoCustom,
        History,
        HistoryNoCustom,
        Smileys,
        People,
        Nature,
        Food,
        Activities,
        Travel,
        Objects,
        Symbols,
        Flags,
        Component,
    };
    Q_ENUM(Category)

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantList history() const;
    Q_INVOKABLE static QVariantList filterModel(const QString &filter, bool limit = true);
    Q_INVOKABLE static QVariantList filterModelNoCustom(const QString &filter, bool limit = true);

    Q_INVOKABLE QVariantList emojis(Category category) const;

    Q_INVOKABLE QVariantList tones(const QString &baseEmoji) const;

    QVariantList categories() const;
    QVariantList categoriesWithCustom() const;

Q_SIGNALS:
    void historyChanged();

public Q_SLOTS:
    void emojiUsed(const QVariant &modelData);

private:
    static QHash<Category, QVariantList> _emojis;

    // TODO: Port away from QSettings
    QSettings m_settings;
    EmojiModel(QObject *parent = nullptr);
};
