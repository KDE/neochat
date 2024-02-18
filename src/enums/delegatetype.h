// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

#include <Quotient/events/encryptedevent.h>
#include <Quotient/events/roomevent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/stickerevent.h>

#include "events/pollevent.h"

/**
 * @class DelegateType
 *
 * This class is designed to define the DelegateType enumeration.
 */
class DelegateType : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief The type of delegate that is needed for the event.
     *
     * @note While similar this is not the matrix event or message type. This is
     *       to tell a QML ListView what delegate to show for each event. So while
     *       similar to the spec it is not the same.
     */
    enum Type {
        Message, /**< A text message. */
        State, /**< A state event in the room. */
        ReadMarker, /**< The local user read marker. */
        Loading, /**< A delegate to tell the user more messages are being loaded. */
        TimelineEnd, /**< A delegate to inform that all messages are loaded. */
        Other, /**< Anything that cannot be classified as another type. */
    };
    Q_ENUM(Type);

    /**
     * @brief Return the delegate type for the given event.
     *
     * @param event the event to return a type for.
     *
     * @sa Type
     */
    static Type typeForEvent(const Quotient::RoomEvent &event)
    {
        if (event.is<Quotient::RoomMessageEvent>() || event.is<Quotient::StickerEvent>() || event.is<Quotient::EncryptedEvent>()
            || event.is<Quotient::PollStartEvent>()) {
            return Message;
        }
        if (event.isStateEvent()) {
            if (event.matrixType() == QStringLiteral("org.matrix.msc3672.beacon_info")) {
                return Message;
            }
            return State;
        }
        return Other;
    }
};
