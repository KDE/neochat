// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QVector>

namespace Quotient
{
class Room;
}

class SpaceHierarchyCache : public QObject
{
    Q_OBJECT

public:
    static SpaceHierarchyCache &instance()
    {
        static SpaceHierarchyCache _instance;
        return _instance;
    }

    [[nodiscard]] QVector<QString> &getRoomListForSpace(const QString &spaceId, bool updateCache);

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
