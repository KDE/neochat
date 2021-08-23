// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>

#include <memory>

#include "connection.h"

using namespace Quotient;

class CustomEmojiModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

public:
    // constructors

    explicit CustomEmojiModel(QObject *parent = nullptr);
    ~CustomEmojiModel();

    // model

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

    // property setters

    Connection *connection() const;
    void setConnection(Connection *it);
    Q_SIGNAL void connectionChanged();

    // QML functions

    Q_INVOKABLE QString preprocessText(const QString &it);
    Q_INVOKABLE QVariantList filterModel(const QString &filter);
    Q_INVOKABLE void addEmoji(const QString &name, const QUrl &location);
    Q_INVOKABLE void removeEmoji(const QString &name);

private:
    struct Private;
    std::unique_ptr<Private> d;

    void fetchEmojies();
};
