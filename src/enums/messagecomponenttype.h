// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
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
 * @class MessageComponentType
 *
 * This class is designed to define the MessageComponentType enumeration.
 */
class MessageComponentType : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief The type of component that is needed for an event.
     *
     * @note While similar this is not the matrix event or message type. This is
     *       to tell a QML Bubble what component to use to visualise all or part of
     *       a room message.
     */
    enum Type {
        Text, /**< A text message. */
        Image, /**< A message that is an image. */
        Audio, /**< A message that is an audio recording. */
        Video, /**< A message that is a video. */
        Code, /**< A code section. */
        Quote, /**< A quote section. */
        File, /**< A message that is a file. */
        Itinerary, /**< A preview for a file that can integrate with KDE itinerary. */
        Pdf, /**< A preview for a PDF file. */
        Poll, /**< The initial event for a poll. */
        Location, /**< A location event. */
        LiveLocation, /**< The initial event of a shared live location (i.e., the place where this is supposed to be shown in the timeline). */
        Encrypted, /**< An encrypted message that cannot be decrypted. */
        Reply, /**< A component to show a replied-to message. */
        ReplyLoad, /**< A loading dialog for a reply. */
        LinkPreview, /**< A preview of a URL in the message. */
        LinkPreviewLoad, /**< A loading dialog for a link preview. */
        Edit, /**< A text edit for editing a message. */
        Verification, /**< A user verification session start message. */
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
        using namespace Quotient;

        if (const auto e = eventCast<const RoomMessageEvent>(&event)) {
            switch (e->msgtype()) {
            case MessageEventType::Emote:
                return MessageComponentType::Text;
            case MessageEventType::Notice:
                return MessageComponentType::Text;
            case MessageEventType::Image:
                return MessageComponentType::Image;
            case MessageEventType::Audio:
                return MessageComponentType::Audio;
            case MessageEventType::Video:
                return MessageComponentType::Video;
            case MessageEventType::Location:
                return MessageComponentType::Location;
            case MessageEventType::File:
                return MessageComponentType::File;
            default:
                return MessageComponentType::Text;
            }
        }
        if (is<const StickerEvent>(event)) {
            return MessageComponentType::Image;
        }
        if (event.isStateEvent()) {
            if (event.matrixType() == QStringLiteral("org.matrix.msc3672.beacon_info")) {
                return MessageComponentType::LiveLocation;
            }
            return MessageComponentType::Other;
        }
        if (is<const EncryptedEvent>(event)) {
            return MessageComponentType::Encrypted;
        }
        if (is<PollStartEvent>(event)) {
            const auto pollEvent = eventCast<const PollStartEvent>(&event);
            if (pollEvent->isRedacted()) {
                return MessageComponentType::Text;
            }
            return MessageComponentType::Poll;
        }

        return MessageComponentType::Other;
    }

    /**
     * @brief Return MessageComponentType for the given html tag.
     *
     * @param tag the tag name to return a type for.
     *
     * @sa Type
     */
    static Type typeForTag(const QString &tag)
    {
        if (tag == QLatin1String("pre") || tag == QLatin1String("pre")) {
            return Code;
        }
        if (tag == QLatin1String("blockquote")) {
            return Quote;
        }
        return Text;
    }
};
