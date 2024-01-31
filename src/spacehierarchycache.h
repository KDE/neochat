
// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QQmlEngine>
#include <QString>

#include "neochatconnection.h"

namespace Quotient
{
class Room;
}

/**
 * @class SpaceHierarchyCache
 *
 * A class to store the child spaces for each space.
 *
 * Spaces are cached on startup or when the user enters a new space.
 */
class SpaceHierarchyCache : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

public:
    static SpaceHierarchyCache &instance()
    {
        static SpaceHierarchyCache _instance;
        return _instance;
    }
    static SpaceHierarchyCache *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    /**
     * @brief Whether the given room is a member of the given space.
     */
    Q_INVOKABLE bool isSpaceChild(const QString &spaceId, const QString &roomId);

    /**
     * @brief Return the list of child rooms for the given space ID.
     */
    [[nodiscard]] QList<QString> &getRoomListForSpace(const QString &spaceId, bool updateCache);

    /**
     * @brief Returns whether the room is a child space of any space.
     *
     * @note We need to do this from the hierarchy as it is not guaranteed that the
     *       child knows it's in a space. See
     *       https://spec.matrix.org/v1.8/client-server-api/#managing-roomsspaces-included-in-a-space
     */
    [[nodiscard]] bool isChild(const QString &roomId) const;

    NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

Q_SIGNALS:
    void spaceHierarchyChanged();
    void connectionChanged();

private Q_SLOTS:
    void addSpaceToHierarchy(Quotient::Room *room);
    void removeSpaceFromHierarchy(Quotient::Room *room);

private:
    explicit SpaceHierarchyCache(QObject *parent = nullptr);

    QList<QString> m_activeSpaceRooms;
    QHash<QString, QList<QString>> m_spaceHierarchy;
    void cacheSpaceHierarchy();
    void populateSpaceHierarchy(const QString &spaceId);
    NeoChatConnection *m_connection;
};
