// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "neochatroom.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QQmlEngine>
#include <QRectF>

struct LiveLocationData {
    QString eventId;
    QString senderId;
    QJsonObject beaconInfo;
    QJsonObject beacon;
};
bool operator<(const LiveLocationData &lhs, const LiveLocationData &rhs);

/** Accumulates live location beacon events in a given room
 *  and provides the last known state for one or more live location beacons.
 */
class LiveLocationsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(NeoChatRoom *room MEMBER m_room NOTIFY roomChanged)
    /** The event id of the beacon start event, ie. the one all suspequent
     *  events use to relate to the same beacon.
     *  If this is set only this specific beacon will be coverd by this model,
     *  if it is empty, all beacons in the room will be covered.
     */
    Q_PROPERTY(QString eventId MEMBER m_eventId NOTIFY eventIdChanged)

    /** Bounding box of all live location beacons covered by this model. */
    Q_PROPERTY(QRectF boundingBox READ boundingBox NOTIFY boundingBoxChanged)

public:
    explicit LiveLocationsModel(QObject *parent = nullptr);

    enum Roles {
        LatitudeRole, /**< Latest latitude of a live locaction beacon. */
        LongitudeRole, /**< Latest longitude of a live locaction beacon. */
        AssetRole, /**< Type of location event, e.g. self pin of the user location. */
        AuthorRole, /**< The author of the event. */
        IsLiveRole, /**< Boolean that indicates whether a live location beacon is still live. */
        HeadingRole, /**< Heading in degree (not part of any MSC yet, using an Itinerary extension). */
    };
    Q_ENUM(Roles)

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int roleName) const override;
    QHash<int, QByteArray> roleNames() const override;

    QRectF boundingBox() const;

Q_SIGNALS:
    void roomChanged();
    void eventIdChanged();
    void boundingBoxChanged();

private:
    void addEvent(const Quotient::RoomEvent *event);
    void updateLocationData(LiveLocationData &&data);

    QPointer<NeoChatRoom> m_room;
    QString m_eventId;

    QList<LiveLocationData> m_locations;
};
