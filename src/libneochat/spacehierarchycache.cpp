// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "spacehierarchycache.h"

#include <Quotient/csapi/space_hierarchy.h>
#include <Quotient/qt_connection_util.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include "neochatconnection.h"
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
            connect(
                neoChatRoom,
                &Room::baseStateLoaded,
                neoChatRoom,
                [this, neoChatRoom]() {
                    if (neoChatRoom->isSpace()) {
                        populateSpaceHierarchy(neoChatRoom->id());
                    }
                },
                Qt::SingleShotConnection);
        }

        connect(neoChatRoom, &NeoChatRoom::changed, this, [this, neoChatRoom](NeoChatRoom::Changes changes) {
            if (neoChatRoom != nullptr && (changes & (NeoChatRoom::Change::UnreadStats | NeoChatRoom::Change::Highlights))) {
                const auto parents = parentSpaces(neoChatRoom->id());
                if (parents.count() > 0) {
                    Q_EMIT spaceNotificationCountChanged(parents);
                }
            }
        });
    }
}

void SpaceHierarchyCache::populateSpaceHierarchy(const QString &spaceId)
{
    if (!m_connection) {
        return;
    }

    m_nextBatchTokens[spaceId] = QString();
    m_connection->callApi<GetSpaceHierarchyJob>(spaceId, std::nullopt, std::nullopt, std::nullopt, *m_nextBatchTokens[spaceId])
        .onResult([this, spaceId](const auto &job) {
            addBatch(spaceId, job);
        });
    auto group = KConfigGroup(KSharedConfig::openStateConfig("SpaceHierarchy"_L1), "Cache"_L1);
    m_spaceHierarchy.insert(spaceId, group.readEntry(spaceId, QStringList()));
}

void SpaceHierarchyCache::addBatch(const QString &spaceId, Quotient::GetSpaceHierarchyJob *job)
{
    const auto rooms = job->rooms();
    QStringList roomList = m_spaceHierarchy[spaceId];
    for (unsigned long i = 0; i < rooms.size(); ++i) {
        for (const auto &state : rooms[i].childrenState) {
            if (!roomList.contains(state->stateKey())) {
                roomList.push_back(state->stateKey());
            }
        }
    }
    m_spaceHierarchy.insert(spaceId, roomList);
    Q_EMIT spaceHierarchyChanged();
    auto group = KConfigGroup(KSharedConfig::openStateConfig("SpaceHierarchy"_L1), "Cache"_L1);
    group.writeEntry(spaceId, roomList);
    group.sync();

    const auto nextBatchToken = job->nextBatch();
    if (!nextBatchToken.isEmpty() && nextBatchToken != *m_nextBatchTokens[spaceId] && m_connection) {
        *m_nextBatchTokens[spaceId] = nextBatchToken;
        m_connection->callApi<GetSpaceHierarchyJob>(spaceId, std::nullopt, std::nullopt, std::nullopt, *m_nextBatchTokens[spaceId])
            .onResult([this, spaceId](const auto &nextJob) {
                addBatch(spaceId, nextJob);
            });
    } else {
        m_nextBatchTokens[spaceId].reset();
    }
}

void SpaceHierarchyCache::addSpaceToHierarchy(Quotient::Room *room)
{
    connect(
        room,
        &Quotient::Room::baseStateLoaded,
        this,
        [this, room]() {
            const auto neoChatRoom = static_cast<NeoChatRoom *>(room);
            if (neoChatRoom->isSpace()) {
                populateSpaceHierarchy(neoChatRoom->id());
            }
        },
        Qt::SingleShotConnection);
}

void SpaceHierarchyCache::removeSpaceFromHierarchy(Quotient::Room *room)
{
    const auto neoChatRoom = static_cast<NeoChatRoom *>(room);
    if (neoChatRoom->isSpace()) {
        m_spaceHierarchy.remove(neoChatRoom->id());
    }
}

QStringList SpaceHierarchyCache::parentSpaces(const QString &roomId)
{
    auto spaces = m_spaceHierarchy.keys();
    QStringList parents;
    for (const auto &space : spaces) {
        if (m_spaceHierarchy[space].contains(roomId)) {
            parents += space;
        }
    }
    return parents;
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

qsizetype SpaceHierarchyCache::notificationCountForSpace(const QString &spaceId)
{
    qsizetype notifications = 0;
    auto children = m_spaceHierarchy[spaceId];
    QStringList added;

    for (const auto &childId : children) {
        if (const auto child = static_cast<NeoChatRoom *>(m_connection->room(childId))) {
            if (!added.contains(child->id())) {
                notifications += child->contextAwareNotificationCount();
                added += child->id();
            }
        }
    }
    return notifications;
}

bool SpaceHierarchyCache::spaceHasHighlightNotifications(const QString &spaceId)
{
    auto children = m_spaceHierarchy[spaceId];
    for (const auto &childId : children) {
        if (const auto child = static_cast<NeoChatRoom *>(m_connection->room(childId))) {
            if (child->highlightCount() > 0) {
                return true;
            }
        }
    }
    return false;
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

bool SpaceHierarchyCache::spaceHasUnreadMessages(const QString &spaceId)
{
    auto children = m_spaceHierarchy[spaceId];

    for (const auto &childId : children) {
        if (const auto child = static_cast<NeoChatRoom *>(m_connection->room(childId))) {
            if (child->notificationCount() > 0) {
                return true;
            }
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

QString SpaceHierarchyCache::recommendedSpaceId() const
{
    return KConfigGroup(KSharedConfig::openConfig(), u"RecommendedSpace"_s).readEntry(u"Id"_s, {});
}

QString SpaceHierarchyCache::recommendedSpaceAvatar() const
{
    return KConfigGroup(KSharedConfig::openConfig(), u"RecommendedSpace"_s).readEntry(u"Avatar"_s, {});
}

QString SpaceHierarchyCache::recommendedSpaceDisplayName() const
{
    return KConfigGroup(KSharedConfig::openConfig(), u"RecommendedSpace"_s).readEntry(u"DisplayName"_s, {});
}

QString SpaceHierarchyCache::recommendedSpaceDescription() const
{
    return KConfigGroup(KSharedConfig::openConfig(), u"RecommendedSpace"_s).readEntry(u"Description"_s, {});
}

bool SpaceHierarchyCache::recommendedSpaceHidden() const
{
    KConfigGroup group(KSharedConfig::openStateConfig(), u"RecommendedSpace"_s);
    return group.readEntry<bool>(u"hidden"_s, false);
}

void SpaceHierarchyCache::setRecommendedSpaceHidden(bool hidden)
{
    KConfigGroup group(KSharedConfig::openStateConfig(), u"RecommendedSpace"_s);
    group.writeEntry(u"hidden"_s, hidden);
    group.sync();
    Q_EMIT recommendedSpaceHiddenChanged();
}

void SpaceHierarchyCache::markAllChildrenMessagesAsRead(const QString &spaceId)
{
    const auto children = m_spaceHierarchy[spaceId];

    for (const auto &childId : children) {
        if (const auto child = static_cast<NeoChatRoom *>(m_connection->room(childId))) {
            if (child->notificationCount() <= 0) {
                continue;
            }

            if (child->messageEvents().crbegin() == child->historyEdge()) {
                if (!child->eventsHistoryJob()) {
                    if (child->allHistoryLoaded()) {
                        continue;
                    }

                    child->getPreviousContent();
                }

                connect(
                    child,
                    &NeoChatRoom::addedMessages,
                    child,
                    [child] {
                        if (child->messageEvents().crbegin() != child->historyEdge()) {
                            child->markAllMessagesAsRead();
                        }
                    },
                    Qt::SingleShotConnection);
            } else {
                child->markAllMessagesAsRead();
            }
        }
    }
}

#include "moc_spacehierarchycache.cpp"
