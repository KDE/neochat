// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "spacehierarchycache.h"

#include <qt_connection_util.h>

#include "controller.h"
#include "neochatroom.h"
#include <csapi/space_hierarchy.h>

using namespace Quotient;

SpaceHierarchyCache::SpaceHierarchyCache(QObject *parent)
    : QObject{parent}
{
    cacheSpaceHierarchy();
    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, [this]() {
        cacheSpaceHierarchy();
        connect(Controller::instance().activeConnection(), &Connection::joinedRoom, this, &SpaceHierarchyCache::addSpaceToHierarchy);
        connect(Controller::instance().activeConnection(), &Connection::aboutToDeleteRoom, this, &SpaceHierarchyCache::removeSpaceFromHierarchy);
    });
}

void SpaceHierarchyCache::cacheSpaceHierarchy()
{
    auto connection = Controller::instance().activeConnection();
    if (!connection) {
        return;
    }

    const auto roomList = connection->allRooms();
    for (const auto &room : roomList) {
        const auto neoChatRoom = static_cast<NeoChatRoom *>(room);
        if (neoChatRoom->isSpace()) {
            populateSpaceHierarchy(neoChatRoom->id());
        } else {
            connect(neoChatRoom, &Room::baseStateLoaded, neoChatRoom, [this, neoChatRoom]() {
                if (neoChatRoom->isSpace()) {
                    populateSpaceHierarchy(neoChatRoom->id());
                }
            });
        }
    }
}

void SpaceHierarchyCache::populateSpaceHierarchy(const QString &spaceId)
{
    auto connection = Controller::instance().activeConnection();
    if (!connection) {
        return;
    }
    GetSpaceHierarchyJob *job = connection->callApi<GetSpaceHierarchyJob>(spaceId);

    connect(job, &BaseJob::success, this, [this, job, spaceId]() {
        const auto rooms = job->rooms();
        QVector<QString> roomList;
        for (unsigned long i = 0; i < rooms.size(); ++i) {
            for (const auto &state : rooms[i].childrenState) {
                roomList.push_back(state->stateKey());
            }
            roomList.push_back(rooms.at(i).roomId);
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

QVector<QString> &SpaceHierarchyCache::getRoomListForSpace(const QString &spaceId, bool updateCache)
{
    if (updateCache) {
        populateSpaceHierarchy(spaceId);
    }
    return m_spaceHierarchy[spaceId];
}
