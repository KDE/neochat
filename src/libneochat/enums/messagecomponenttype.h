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

using namespace Qt::StringLiterals;

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
        Author, /**< The message sender and time. */
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
        Reaction, /**< A component to show the reactions to this message. */
        LinkPreview, /**< A preview of a URL in the message. */
        LinkPreviewLoad, /**< A loading dialog for a link preview. */
        ChatBar, /**< A text edit for editing a message. */
        ThreadRoot, /**< The root message of the thread. */
        ThreadBody, /**< The other messages in the thread. */
        ReplyButton, /**< A button to reply in the current thread. */
        FetchButton, /**< A button to fetch more messages in the current thread. */
        Verification, /**< A user verification session start message. */
        Loading, /**< The component is loading. */
        Separator, /**< A horizontal separator. */
        Other, /**< Anything that cannot be classified as another type. */
    };
    Q_ENUM(Type);

    /**
     * @brief Return the delegate type for the given event.
     *
     * @param event the event to return a type for.
     *
     * @param isInReply whether this event is to be treated like a replied-to event (i.e., a basic text fallback should be shown if no other type is used)
     *
     * @sa Type
     */
    static Type typeForEvent(const Quotient::RoomEvent &event, bool isInReply = false)
    {
        using namespace Quotient;

        if (event.isRedacted()) {
            return MessageComponentType::Text;
        }

        if (const auto e = eventCast<const RoomMessageEvent>(&event)) {
            if (e->rawMsgtype() == u"m.key.verification.request"_s) {
                return MessageComponentType::Verification;
            }

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
            if (event.matrixType() == u"org.matrix.msc3672.beacon_info"_s) {
                return MessageComponentType::LiveLocation;
            }
            // In the (unlikely) case that this is a reply to a state event, we do want to show something
            return isInReply ? MessageComponentType::Text : MessageComponentType::Other;
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

        // In the (unlikely) case that this is a reply to an unusual event, we do want to show something
        return isInReply ? MessageComponentType::Text : MessageComponentType::Other;
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
        if (tag == u"pre"_s || tag == u"pre"_s) {
            return Code;
        }
        if (tag == u"blockquote"_s) {
            return Quote;
        }
        return Text;
    }
};
