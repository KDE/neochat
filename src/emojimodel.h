/**
 * SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QVector>
#include <utility>

struct Emoji {
    Emoji(QString u, QString s)
        : unicode(std::move(std::move(u)))
        , shortname(std::move(std::move(s)))
    {
    }
    Emoji() = default;

    friend QDataStream &operator<<(QDataStream &arch, const Emoji &object)
    {
        arch << object.unicode;
        arch << object.shortname;
        return arch;
    }

    friend QDataStream &operator>>(QDataStream &arch, Emoji &object)
    {
        arch >> object.unicode;
        arch >> object.shortname;
        return arch;
    }

    QString unicode;
    QString shortname;

    Q_GADGET
    Q_PROPERTY(QString unicode MEMBER unicode)
    Q_PROPERTY(QString shortname MEMBER shortname)
};

Q_DECLARE_METATYPE(Emoji)

class EmojiModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)

    Q_PROPERTY(QVariantList people MEMBER people CONSTANT)
    Q_PROPERTY(QVariantList nature MEMBER nature CONSTANT)
    Q_PROPERTY(QVariantList food MEMBER food CONSTANT)
    Q_PROPERTY(QVariantList activity MEMBER activity CONSTANT)
    Q_PROPERTY(QVariantList travel MEMBER travel CONSTANT)
    Q_PROPERTY(QVariantList objects MEMBER objects CONSTANT)
    Q_PROPERTY(QVariantList symbols MEMBER symbols CONSTANT)
    Q_PROPERTY(QVariantList flags MEMBER flags CONSTANT)

public:
    explicit EmojiModel(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    Q_INVOKABLE QVariantList history();
    Q_INVOKABLE static QVariantList filterModel(const QString &filter);

Q_SIGNALS:
    void historyChanged();

public Q_SLOTS:
    void emojiUsed(const QVariant &modelData);

private:
    static const QVariantList people;
    static const QVariantList nature;
    static const QVariantList food;
    static const QVariantList activity;
    static const QVariantList travel;
    static const QVariantList objects;
    static const QVariantList symbols;
    static const QVariantList flags;

    QSettings m_settings;
};
