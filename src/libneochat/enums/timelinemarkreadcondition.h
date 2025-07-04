// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

/**
 * @class TimelineMarkReadCondition
 *
 * This class is designed to define the TimelineMarkReadCondition enumeration.
 */
class TimelineMarkReadCondition : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief The condition for marking messages as read.
     */
    enum Condition {
        Never = 0, /**< Messages should never be marked automatically. */
        Entry, /**< Messages should be marked automatically on entry to the room. */
        EntryVisible, /**< Messages should be marked automatically on entry to the room if all messages are visible. */
        Exit, /**< Messages should be marked automatically on exiting the room. */
        ExitVisible, /**< Messages should be marked automatically on exiting the room if all messages are visible. */
    };
    Q_ENUM(Condition);
};
