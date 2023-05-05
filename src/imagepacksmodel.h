// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "imagepackevent.h"
#include <QAbstractListModel>
#include <QPointer>
#include <QVector>

class NeoChatRoom;

class ImagePacksModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)
    Q_PROPERTY(bool showStickers READ showStickers WRITE setShowStickers NOTIFY showStickersChanged)
    Q_PROPERTY(bool showEmoticons READ showEmoticons WRITE setShowEmoticons NOTIFY showEmoticonsChanged)

public:
    enum Roles {
        DisplayNameRole = Qt::DisplayRole,
        AvatarUrlRole,
        AttributionRole,
        IdRole,
    };
    Q_ENUM(Roles);

    explicit ImagePacksModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &index) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    [[nodiscard]] bool showStickers() const;
    void setShowStickers(bool showStickers);

    [[nodiscard]] bool showEmoticons() const;
    void setShowEmoticons(bool showEmoticons);

    [[nodiscard]] QVector<Quotient::ImagePackEventContent::ImagePackImage> images(int index);

Q_SIGNALS:
    void roomChanged();
    void showStickersChanged();
    void showEmoticonsChanged();

private:
    QPointer<NeoChatRoom> m_room;
    QVector<Quotient::ImagePackEventContent> m_events;
    bool m_showStickers = true;
    bool m_showEmoticons = true;
};
