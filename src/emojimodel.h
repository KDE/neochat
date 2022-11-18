// SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QSettings>

struct Emoji {
    Emoji(QString u, QString s, bool isCustom = false)
        : unicode(std::move(std::move(u)))
        , shortName(std::move(std::move(s)))
        , isCustom(isCustom)
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
    bool isCustom = false;

    Q_GADGET
    Q_PROPERTY(QString unicode MEMBER unicode)
    Q_PROPERTY(QString shortName MEMBER shortName)
    Q_PROPERTY(bool isCustom MEMBER isCustom)
};

Q_DECLARE_METATYPE(Emoji)

class EmojiModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)
    Q_PROPERTY(QVariantList categories READ categories CONSTANT)

public:
    explicit EmojiModel(QObject *parent = nullptr);

    enum RoleNames {
        TextRole = Qt::DisplayRole,
        UnicodeRole,
    };
    Q_ENUM(RoleNames);

    enum Category {
        Custom,
        Search,
        History,
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
    Q_INVOKABLE QVariantList emojis(Category category) const;

    Q_INVOKABLE QVariantList tones(const QString &baseEmoji) const;

    QVariantList categories() const;

Q_SIGNALS:
    void historyChanged();

public Q_SLOTS:
    void emojiUsed(const QVariant &modelData);

private:
    static QHash<Category, QVariantList> _emojis;
    static QMultiHash<QString, QVariant> _tones;

    // TODO: Port away from QSettings
    QSettings m_settings;
};
