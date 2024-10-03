// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

/**
 * @class MessageType
 *
 * This class is designed to define the MessageType enumeration.
 */
class MessageType : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief The types of messages that can be shown.
     */
    enum Type {
        Information = 0, /**< Info message, typically highlight color. */
        Positive, /**< Positive message, typically green. */
        Warning, /**< Warning message, typically amber. */
        Error, /**< Error message, typically red. */
    };
    Q_ENUM(Type);
};
