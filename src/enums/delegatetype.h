// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

/**
 * @class DelegateType
 *
 * This class is designed to define the DelegateType enumeration.
 */
class DelegateType
{
    Q_GADGET

public:
    /**
     * @brief The type of delegate that is needed for the event.
     *
     * @note While similar this is not the matrix event or message type. This is
     *       to tell a QML ListView what delegate to show for each event. So while
     *       similar to the spec it is not the same.
     */
    enum Type {
        Emote, /**< A message that begins with /me. */
        Notice, /**< A notice event. */
        Image, /**< A message that is an image. */
        Audio, /**< A message that is an audio recording. */
        Video, /**< A message that is a video. */
        File, /**< A message that is a file. */
        Message, /**< A text message. */
        Sticker, /**< A message that is a sticker. */
        State, /**< A state event in the room. */
        Encrypted, /**< An encrypted message that cannot be decrypted. */
        ReadMarker, /**< The local user read marker. */
        Poll, /**< The initial event for a poll. */
        Location, /**< A location event. */
        LiveLocation, /**< The initial event of a shared live location (i.e., the place where this is supposed to be shown in the timeline). */
        Other, /**< Anything that cannot be classified as another type. */
    };
    Q_ENUM(Type);
};
