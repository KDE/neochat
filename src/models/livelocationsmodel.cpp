// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "livelocationsmodel.h"

#include <Quotient/events/roommessageevent.h>

#include <QDebug>

#include <cmath>

using namespace Quotient;

bool operator<(const LiveLocationData &lhs, const LiveLocationData &rhs)
{
    return lhs.eventId < rhs.eventId;
}

LiveLocationsModel::LiveLocationsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(
        this,
        &LiveLocationsModel::roomChanged,
        this,
        [this]() {
            for (const auto &event : m_room->messageEvents()) {
                addEvent(event.get());
            }
            connect(m_room, &NeoChatRoom::aboutToAddHistoricalMessages, this, [this](const auto &events) {
                for (const auto &event : events) {
                    addEvent(event.get());
                }
            });
            connect(m_room, &NeoChatRoom::aboutToAddNewMessages, this, [this](const auto &events) {
                for (const auto &event : events) {
                    addEvent(event.get());
                }
            });
        },
        Qt::QueuedConnection); // deferred so we are sure the eventId filter is set

    connect(this, &LiveLocationsModel::dataChanged, this, &LiveLocationsModel::boundingBoxChanged);
    connect(this, &LiveLocationsModel::rowsInserted, this, &LiveLocationsModel::boundingBoxChanged);
}

int LiveLocationsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_locations.size();
}

QVariant LiveLocationsModel::data(const QModelIndex &index, int roleName) const
{
    if (!checkIndex(index)) {
        return {};
    }

    const auto &data = m_locations.at(index.row());
    switch (roleName) {
    case LatitudeRole: {
        const auto geoUri = data.beacon["org.matrix.msc3488.location"_ls].toObject()["uri"_ls].toString();
        if (geoUri.isEmpty()) {
            return {};
        }
        const auto latitude = geoUri.split(u';')[0].split(u':')[1].split(u',')[0];
        return latitude.toFloat();
    }
    case LongitudeRole: {
        const auto geoUri = data.beacon["org.matrix.msc3488.location"_ls].toObject()["uri"_ls].toString();
        if (geoUri.isEmpty()) {
            return {};
        }
        const auto longitude = geoUri.split(u';')[0].split(u':')[1].split(u',')[1];
        return longitude.toFloat();
    }
    case AssetRole:
        return data.beaconInfo["org.matrix.msc3488.asset"_ls].toObject()["type"_ls].toString();
    case AuthorRole:
        return m_room->getUser(data.senderId);
    case IsLiveRole: {
        if (!data.beaconInfo["live"_ls].toBool()) {
            return false;
        }
        // TODO Qt6: port to toInteger(), timestamps are in ms since epoch, ie. 64 bit values
        const auto lastTs = std::max(data.beaconInfo.value("org.matrix.msc3488.ts"_ls).toDouble(), data.beacon.value("org.matrix.msc3488.ts"_ls).toDouble());
        const auto timeout = data.beaconInfo.value("timeout"_ls).toDouble(600000);
        return lastTs + timeout >= QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
    case HeadingRole: {
        bool success = false;
        const auto heading = data.beacon["org.matrix.msc3488.location"_ls].toObject()["org.kde.itinerary.heading"_ls].toString().toDouble(&success);
        return success ? heading : NAN;
    }
    }

    return {};
}

QHash<int, QByteArray> LiveLocationsModel::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(LatitudeRole, "latitude");
    r.insert(LongitudeRole, "longitude");
    r.insert(AssetRole, "asset");
    r.insert(AuthorRole, "author");
    r.insert(IsLiveRole, "isLive");
    r.insert(HeadingRole, "heading");
    return r;
}

QRectF LiveLocationsModel::boundingBox() const
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

void LiveLocationsModel::addEvent(const Quotient::RoomEvent *event)
{
    if (event->isStateEvent() && event->matrixType() == "org.matrix.msc3672.beacon_info"_ls) {
        LiveLocationData data;
        data.senderId = event->senderId();
        data.beaconInfo = event->contentJson();
        if (event->contentJson()["live"_ls].toBool()) {
            data.eventId = event->id();
        } else {
            data.eventId = event->fullJson()["replaces_state"_ls].toString();
        }
        updateLocationData(std::move(data));
    }
    if (event->matrixType() == "org.matrix.msc3672.beacon"_ls) {
        LiveLocationData data;
        data.eventId = event->contentJson()["m.relates_to"_ls].toObject()["event_id"_ls].toString();
        data.senderId = event->senderId();
        data.beacon = event->contentJson();
        updateLocationData(std::move(data));
    }
}

void LiveLocationsModel::updateLocationData(LiveLocationData &&data)
{
    if (!m_eventId.isEmpty() && data.eventId != m_eventId) {
        return;
    }

    auto it = std::lower_bound(m_locations.begin(), m_locations.end(), data);
    if (it == m_locations.end() || it->eventId != data.eventId) {
        const auto row = std::distance(m_locations.begin(), it);
        beginInsertRows({}, row, row);
        m_locations.insert(it, std::move(data));
        endInsertRows();
        return;
    }

    const auto idx = index(std::distance(m_locations.begin(), it), 0);

    // TODO Qt6: port to toInteger(), timestamps are in ms since epoch, ie. 64 bit values
    if (it->beacon.isEmpty() || it->beacon.value("org.matrix.msc3488.ts"_ls).toDouble() < data.beacon.value("org.matrix.msc3488.ts"_ls).toDouble()) {
        it->beacon = std::move(data.beacon);
    }
    if (it->beaconInfo.isEmpty()
        || it->beaconInfo.value("org.matrix.msc3488.ts"_ls).toDouble() < data.beaconInfo.value("org.matrix.msc3488.ts"_ls).toDouble()) {
        it->beaconInfo = std::move(data.beaconInfo);
    }

    Q_EMIT dataChanged(idx, idx);
}

#include "moc_livelocationsmodel.cpp"
