// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

namespace Quotient
{
class RoomEvent;
}

using namespace Qt::StringLiterals;

namespace Blocks
{
Q_NAMESPACE
QML_ELEMENT

/**
 * @brief The type of a block.
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
Q_ENUM_NS(Type);

/**
 * @brief Return the delegate type for the given event.
 *
 * @param event the event to return a type for.
 *
 * @param isInReply whether this event is to be treated like a replied-to event (i.e., a basic text fallback should be shown if no other type is used)
 *
 * @sa Type
 */
Type typeForEvent(const Quotient::RoomEvent &event, bool isInReply = false);

/**
 * @brief Return Blocks::Type for the given html tag.
 *
 * @param tag the tag name to return a type for.
 *
 * @sa Type
 */
Type typeForTag(const QString &tag);

/**
 * @brief Return Blocks::Type for the file with the given path.
 *
 * @sa Type
 */
Type typeForPath(const QUrl &path);

/**
 * @brief Return if the given Blocks::Type is a text type.
 *
 * @sa Type
 */
bool isTextType(const Type &type);

/**
 * @brief Return if the given Blocks::Type is a file type.
 *
 * @sa Type
 */
bool isFileType(const Type &type);
}
