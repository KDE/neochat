// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "spacehierarchycache.h"

#include <Quotient/csapi/space_hierarchy.h>
#include <Quotient/qt_connection_util.h>

#include "neochatroom.h"

using namespace Quotient;

SpaceHierarchyCache::SpaceHierarchyCache(QObject *parent)
    : QObject{parent}
{
}

void SpaceHierarchyCache::cacheSpaceHierarchy()
{
    if (!m_connection) {
        return;
    }

    const auto &roomList = m_connection->allRooms();
    for (const auto &room : roomList) {
        const auto neoChatRoom = static_cast<NeoChatRoom *>(room);
        if (neoChatRoom->isSpace()) {
            populateSpaceHierarchy(neoChatRoom->id());
        } else {
            connectSingleShot(neoChatRoom, &Room::baseStateLoaded, neoChatRoom, [this, neoChatRoom]() {
                if (neoChatRoom->isSpace()) {
                    populateSpaceHierarchy(neoChatRoom->id());
                }
            });
        }
    }
}

void SpaceHierarchyCache::populateSpaceHierarchy(const QString &spaceId)
{
    if (!m_connection) {
        return;
    }
    auto job = m_connection->callApi<GetSpaceHierarchyJob>(spaceId);

    connect(job, &BaseJob::success, this, [this, job, spaceId]() {
        const auto rooms = job->rooms();
        QList<QString> roomList;
        for (unsigned long i = 0; i < rooms.size(); ++i) {
            for (const auto &state : rooms[i].childrenState) {
                roomList.push_back(state->stateKey());
            }
        }
        m_spaceHierarchy.insert(spaceId, roomList);
        Q_EMIT spaceHierarchyChanged();
    });
}

void SpaceHierarchyCache::addSpaceToHierarchy(Quotient::Room *room)
{
    connectSingleShot(room, &Quotient::Room::baseStateLoaded, this, [this, room]() {
        const auto neoChatRoom = static_cast<NeoChatRoom *>(room);
        if (neoChatRoom->isSpace()) {
            populateSpaceHierarchy(neoChatRoom->id());
        }
    });
}

void SpaceHierarchyCache::removeSpaceFromHierarchy(Quotient::Room *room)
{
    const auto neoChatRoom = static_cast<NeoChatRoom *>(room);
    if (neoChatRoom->isSpace()) {
        m_spaceHierarchy.remove(neoChatRoom->id());
    }
}

bool SpaceHierarchyCache::isSpaceChild(const QString &spaceId, const QString &roomId)
{
    return getRoomListForSpace(spaceId, false).contains(roomId);
}

QList<QString> &SpaceHierarchyCache::getRoomListForSpace(const QString &spaceId, bool updateCache)
{
    if (updateCache) {
        populateSpaceHierarchy(spaceId);
    }
    return m_spaceHierarchy[spaceId];
}

bool SpaceHierarchyCache::isChild(const QString &roomId) const
{
    const auto childrens = m_spaceHierarchy.values();
    for (const auto &children : childrens) {
        if (children.contains(roomId)) {
            return true;
        }
    }
    return false;
}

NeoChatConnection *SpaceHierarchyCache::connection() const
{
    return m_connection;
}

void SpaceHierarchyCache::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }
    m_connection = connection;
    Q_EMIT connectionChanged();
    m_spaceHierarchy.clear();
    cacheSpaceHierarchy();
    connect(connection, &Connection::joinedRoom, this, &SpaceHierarchyCache::addSpaceToHierarchy);
    connect(connection, &Connection::aboutToDeleteRoom, this, &SpaceHierarchyCache::removeSpaceFromHierarchy);
}

#include "moc_spacehierarchycache.cpp"
