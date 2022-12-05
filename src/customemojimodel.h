// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <memory>
#include <QRegularExpression>

struct CustomEmoji {
    QString name; // with :semicolons:
    QString url; // mxc://
    QRegularExpression regexp;

    Q_GADGET
    Q_PROPERTY(QString unicode MEMBER url)
    Q_PROPERTY(QString name MEMBER name)
};

class CustomEmojiModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        Name = Qt::DisplayRole,
        ImageURL,
        ModelData, // for emulating the regular emoji model's usage, otherwise the UI code would get too complicated
        MxcUrl = 50,
        DisplayRole = 51,
        ReplacedTextRole = 52,
    };
    Q_ENUM(Roles);

    static CustomEmojiModel &instance()
    {
        static CustomEmojiModel _instance;
        return _instance;
    }

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QString preprocessText(const QString &it);
    Q_INVOKABLE QVariantList filterModel(const QString &filter);
    Q_INVOKABLE void addEmoji(const QString &name, const QUrl &location);
    Q_INVOKABLE void removeEmoji(const QString &name);

private:
    explicit CustomEmojiModel(QObject *parent = nullptr);
    QList<CustomEmoji> m_emojis;

    void fetchEmojis();
};
