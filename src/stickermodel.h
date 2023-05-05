// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "imagepackevent.h"
#include "neochatroom.h"
#include <QAbstractListModel>
#include <QObject>
#include <QVector>

class ImagePacksModel;

class StickerModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(ImagePacksModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int packIndex READ packIndex WRITE setPackIndex NOTIFY packIndexChanged)
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    enum Roles {
        Url = Qt::UserRole + 1,
        Body,
    };

    explicit StickerModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &index) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] ImagePacksModel *model() const;
    void setModel(ImagePacksModel *model);

    [[nodiscard]] int packIndex() const;
    void setPackIndex(int index);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    Q_INVOKABLE void postSticker(int index);

Q_SIGNALS:
    void roomChanged();
    void modelChanged();
    void packIndexChanged();

private:
    ImagePacksModel *m_model = nullptr;
    int m_index = 0;
    QVector<Quotient::ImagePackEventContent::ImagePackImage> m_images;
    NeoChatRoom *m_room;
};
