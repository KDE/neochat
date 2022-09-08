// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "spacehierarchycache.h"

#include "controller.h"
#ifdef QUOTIENT_07
#include <csapi/space_hierarchy.h>
#endif
#include "neochatroom.h"

SpaceHierarchyCache::SpaceHierarchyCache(QObject *parent)
    : QObject{parent}
{
    cacheSpaceHierarchy();
    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, [this]() {
        cacheSpaceHierarchy();
    });
}

void SpaceHierarchyCache::cacheSpaceHierarchy()
{
#ifdef QUOTIENT_07
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
#endif
}

void SpaceHierarchyCache::populateSpaceHierarchy(const QString &spaceId)
{
    auto connection = Controller::instance().activeConnection();
    if (!connection) {
        return;
    }
#ifdef QUOTIENT_07
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
#endif
}

QVector<QString> SpaceHierarchyCache::getRoomListForSpace(const QString &spaceId, bool updateCache)
{
    if (updateCache) {
        populateSpaceHierarchy(spaceId);
    }
    return m_spaceHierarchy[spaceId];
}
