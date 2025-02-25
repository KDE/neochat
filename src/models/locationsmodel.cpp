// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "locationsmodel.h"

#include <QGuiApplication>

using namespace Quotient;

LocationsModel::LocationsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &LocationsModel::roomChanged, this, [this]() {
        for (const auto &event : m_room->messageEvents()) {
            if (!is<RoomMessageEvent>(*event)) {
                continue;
            }
            if (event->contentJson()["msgtype"_L1] == "m.location"_L1) {
                const auto &e = *event;
                addLocation(eventCast<const RoomMessageEvent>(&e));
            }
        }
        connect(m_room, &NeoChatRoom::aboutToAddHistoricalMessages, this, [this](const auto &events) {
            for (const auto &event : events) {
                if (!is<RoomMessageEvent>(*event)) {
                    continue;
                }
                if (event->contentJson()["msgtype"_L1] == "m.location"_L1) {
                    const auto &e = *event;
                    addLocation(eventCast<const RoomMessageEvent>(&e));
                }
            }
        });
        connect(m_room, &NeoChatRoom::aboutToAddNewMessages, this, [this](const auto &events) {
            for (const auto &event : events) {
                if (!is<RoomMessageEvent>(*event)) {
                    continue;
                }
                if (event->contentJson()["msgtype"_L1] == "m.location"_L1) {
                    const auto &e = *event;
                    addLocation(eventCast<const RoomMessageEvent>(&e));
                }
            }
        });
    });

    connect(this, &LocationsModel::rowsInserted, this, &LocationsModel::boundingBoxChanged);
}

void LocationsModel::addLocation(const RoomMessageEvent *event)
{
    const auto uri = event->contentJson()["org.matrix.msc3488.location"_L1]["uri"_L1].toString();
    const auto parts = uri.mid(4).split(QLatin1Char(','));
    if (parts.size() < 2) {
        qWarning() << "invalid geo: URI" << uri;
        return;
    }
    const auto latitude = parts[0].toFloat();
    const auto longitude = parts[1].toFloat();
    beginInsertRows(QModelIndex(), m_locations.size(), m_locations.size() + 1);
    m_locations += LocationData{
        .eventId = event->id(),
        .latitude = latitude,
        .longitude = longitude,
        .content = event->contentJson(),
        .member = m_room->member(event->senderId()),
    };
    endInsertRows();
}

NeoChatRoom *LocationsModel::room() const
{
    return m_room;
}

void LocationsModel::setRoom(NeoChatRoom *room)
{
    if (m_room) {
        disconnect(this, nullptr, m_room, nullptr);
    }
    m_room = room;
    Q_EMIT roomChanged();
}

QHash<int, QByteArray> LocationsModel::roleNames() const
{
    return {
        {LongitudeRole, "longitude"},
        {LatitudeRole, "latitude"},
        {TextRole, "text"},
        {AssetRole, "asset"},
        {AuthorRole, "author"},
    };
}

QVariant LocationsModel::data(const QModelIndex &index, int roleName) const
{
    auto row = index.row();
    if (roleName == LongitudeRole) {
        return m_locations[row].longitude;
    } else if (roleName == LatitudeRole) {
        return m_locations[row].latitude;
    } else if (roleName == TextRole) {
        return m_locations[row].content["body"_L1].toString();
    } else if (roleName == AssetRole) {
        return m_locations[row].content["org.matrix.msc3488.asset"_L1].toObject()["type"_L1].toString();
    } else if (roleName == AuthorRole) {
        return QVariant::fromValue(m_locations[row].member);
    }
    return {};
}

int LocationsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_locations.size();
}

QRectF LocationsModel::boundingBox() const
{
    QRectF bbox(QPointF(180.0, 90.0), QPointF(-180.0, -90.0));
    for (auto i = 0; i < rowCount(); ++i) {
        const auto lat = data(index(i, 0), LatitudeRole).toDouble();
        const auto lon = data(index(i, 0), LongitudeRole).toDouble();

        bbox.setLeft(std::min(bbox.left(), lon));
        bbox.setRight(std::max(bbox.right(), lon));
        bbox.setTop(std::min(bbox.top(), lat));
        bbox.setBottom(std::max(bbox.bottom(), lat));
    }
    return bbox;
}

bool LocationsModel::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole});
    }
    return QObject::event(event);
}

#include "moc_locationsmodel.cpp"
