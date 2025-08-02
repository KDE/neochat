// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

/**
 * @class ChatBarType
 *
 * This class is designed to define the ChatBarType enumeration.
 */
class ChatBarType : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief The type of chatbar.
     */
    enum Type {
        Room = 0, /**< A standard room chatbar for creating new messages. */
        Edit, /**< A chatbar for editing an existing message. */
        Thread, /**< A chatbar for creating a new threaded message. */
        None, /**< Undefined. */
    };
    Q_ENUM(Type);
};
