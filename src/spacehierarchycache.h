// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QHash>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QVector>

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

public:
    static SpaceHierarchyCache &instance()
    {
        static SpaceHierarchyCache _instance;
        return _instance;
    }
    static SpaceHierarchyCache *create(QQmlEngine *, QJSEngine *)
    {
        return &instance();
    }

    /**
     * @brief Return the list of child rooms for the given space ID.
     */
    [[nodiscard]] QVector<QString> &getRoomListForSpace(const QString &spaceId, bool updateCache);

    /**
     * @brief Returns whether the space is a child space of any other space.
     */
    [[nodiscard]] bool isChildSpace(const QString &spaceId) const;

Q_SIGNALS:
    void spaceHierarchyChanged();

private Q_SLOTS:
    void addSpaceToHierarchy(Quotient::Room *room);
    void removeSpaceFromHierarchy(Quotient::Room *room);

private:
    explicit SpaceHierarchyCache(QObject *parent = nullptr);

    QVector<QString> m_activeSpaceRooms;
    QHash<QString, QVector<QString>> m_spaceHierarchy;
    void cacheSpaceHierarchy();
    void populateSpaceHierarchy(const QString &spaceId);
};
