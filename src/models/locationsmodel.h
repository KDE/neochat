// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QPointer>
#include <QQmlEngine>
#include <QRectF>

#include "neochatroom.h"

#include <Quotient/events/roommessageevent.h>
#include <Quotient/user.h>

class LocationsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        TextRole = Qt::DisplayRole,
        LongitudeRole,
        LatitudeRole,
        AssetRole,
        AuthorRole,
    };
    Q_ENUM(Roles)
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)
    /** Bounding box of all locations covered by this model. */
    Q_PROPERTY(QRectF boundingBox READ boundingBox NOTIFY boundingBoxChanged)

    explicit LocationsModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    QRectF boundingBox() const;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int roleName) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;

Q_SIGNALS:
    void roomChanged();
    void boundingBoxChanged();

protected:
    bool event(QEvent *event) override;

private:
    QPointer<NeoChatRoom> m_room;

    struct LocationData {
        QString eventId;
        float latitude;
        float longitude;
        QJsonObject content;
        QString author;
    };
    QList<LocationData> m_locations;
    void addLocation(const Quotient::RoomMessageEvent *event);
};
