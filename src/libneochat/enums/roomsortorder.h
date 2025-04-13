// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

/**
 * @class RoomSortOrder
 *
 * This class is designed to define the RoomSortOrder enumeration.
 */
class RoomSortOrder : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief The types of messages that can be shown.
     */
    enum Order {
        Alphabetical = 0, /**< The room should be sorted alphabetically. */
        Activity, /**< The room should be sorted by important activity. */
        LastMessage, /**< The room should be sorted by last message in the room. */
        Custom, /**< Use a custom sort order. */
    };
    Q_ENUM(Order);
};
