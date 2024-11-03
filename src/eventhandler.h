// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QDateTime>
#include <QString>
#include <Quotient/events/eventcontent.h>

namespace Quotient
{
namespace EventContent
{
class FileInfo;
}
class RoomEvent;
class RoomMember;
class RoomMessageEvent;
}

class NeoChatRoom;

/**
 * @class EventHandler
 *
 * This class is designed to handle a Quotient::RoomEvent allowing data to be extracted
 * in a form ready for the NeoChat UI.
 *
 * To use this properly both the room and the event should be set (and the event should
 * be from the given room).
 *
 * @note EventHandler will always try to return something even when not properly
 *       initialised, this is usually the best empty value it can create with available
 *       information. This is to minimize warnings from QML especially during startup
 *       and room changes.
 */
class EventHandler
{
public:
    /**
     * @brief Return the ID of the event.
     *
     * Returns the transaction ID if the Matrix ID is empty, which may be the case
     * for a pending event.
     */
    static QString id(const Quotient::RoomEvent *event);

    /**
     * @brief Get the display name of the event author.
     *
     * This method is special in that it will return
     * the old display name of the author if the current event is one that caused it
     * to change. This allows for scenarios where the UI wishes to notify that a
     * user's display name has changed and what it changed from.
     *
     * @param isPending whether the event is pending as this cannot be derived from
     *                  just the event object.
     */
    static QString authorDisplayName(const NeoChatRoom *room, const Quotient::RoomEvent *event, bool isPending = false);

    /**
     * @brief Get the display name of the event author but with any newlines removed.
     *
     * Turns out you can put newlines in your display name so we need to handle that
     * primarily for the room list subtitle.
     *
     * @param isPending whether the event is pending as this cannot be derived from
     *                  just the event object.
     */
    static QString singleLineAuthorDisplayname(const NeoChatRoom *room, const Quotient::RoomEvent *event, bool isPending = false);

    /**
     * @brief Return a QDateTime object for the event timestamp.
     */
    static QDateTime time(const Quotient::RoomEvent *event, bool isPending = false, QDateTime lastUpdated = {});

    /**
     * @brief Return a QString for the event timestamp.
     *
     * This is intended to return a string that is read for display in the UI without
     * any further manipulation required.
     *
     * @param relative whether the string is realtive to the current date, i.e.
     *                 Yesterday or Wednesday, etc.
     * @param format the QLocale::FormatType to use.
     * @param isPending whether the event is pending as this cannot be derived from
     *                  just the event object.
     * @param lastUpdated the time the event was last updated locally as this cannot be
     *                    obtained from the event.
     */
    static QString timeString(const Quotient::RoomEvent *event,
                              bool relative,
                              QLocale::FormatType format = QLocale::ShortFormat,
                              bool isPending = false,
                              QDateTime lastUpdated = {});

    /**
     * @brief Return a QString for the event timestamp.
     *
     * This is intended to return a string that is read for display in the UI without
     * any further manipulation required.
     *
     * @param format the format to use as a string.
     * @param isPending whether the event is pending as this cannot be derived from
     *                  just the event object.
     * @param lastUpdated the time the event was last updated locally as this cannot be
     *                    obtained from the event.
     */
    static QString timeString(const Quotient::RoomEvent *event, const QString &format, bool isPending = false, const QDateTime &lastUpdated = {});

    /**
     * @brief Whether the event should be highlighted in the timeline.
     *
     * @note Messages in direct chats are never highlighted.
     */
    static bool isHighlighted(const NeoChatRoom *room, const Quotient::RoomEvent *event);

    /**
     * @brief Whether the event should be hidden in the timeline.
     *
     * This could be for numerous reasons, e.g. if it's a replacement event, if the
     * user has hidden all state events or if the sender has been ignored by the local
     * user.
     */
    static bool isHidden(const NeoChatRoom *room, const Quotient::RoomEvent *event);

    /**
     * @brief The input format of the body in the message.
     *
     * I.e. if the message has only a body the format will be Qt::PlainText, if it
     * has a formatted body it will be Qt::RichText.
     */
    static Qt::TextFormat messageBodyInputFormat(const Quotient::RoomMessageEvent &event);

    /**
     * @brief Output a string for the room message content without any formatting.
     *
     * This is the content of the formatted_body key if present or the body key if
     * not.
     */
    static QString rawMessageBody(const Quotient::RoomMessageEvent &event);

    /**
     * @brief Output a string for the message content ready for display in a rich text field.
     *
     * The output string is dependant upon the event type and the desired output format.
     *
     * For most messages this is the body content of the message. For media messages
     * this will be the caption and for state events it will be a string specific
     * to that event with some dynamic details about the event added.
     *
     * E.g. For a room topic state event the text will be:
     *      "set the topic to: <new topic text>"
     *
     * @param stripNewlines whether the output should have new lines in it.
     */
    static QString richBody(const NeoChatRoom *room, const Quotient::RoomEvent *event, bool stripNewlines = false);

    /**
     * @brief Output a string for the message content ready for display in a plain text field.
     *
     * The output string is dependant upon the event type and the desired output format.
     *
     * For most messages this is the body content of the message. For media messages
     * this will be the caption and for state events it will be a string specific
     * to that event with some dynamic details about the event added.
     *
     * E.g. For a room topic state event the text will be:
     *      "set the topic to: <new topic text>"
     *
     * @param stripNewlines whether the output should have new lines in it.
     */
    static QString plainBody(const NeoChatRoom *room, const Quotient::RoomEvent *event, bool stripNewlines = false);

    /**
     * @brief Output the original body for the message content, useful for editing the original message.
     *
     * The event type must be a room message event.
     */
    static QString markdownBody(const Quotient::RoomEvent *event);

    /**
     * @brief Output a generic string for the message content ready for display.
     *
     * The output string is dependant upon the event type.
     *
     * Unlike EventHandler::getRichBody or EventHandler::getPlainBody the string
     * is the same for all events of the same type.
     *
     * E.g. For a message the text will be:
     *      "sent a message"
     *
     * @sa richBody(), plainBody()
     */
    static QString genericBody(const NeoChatRoom *room, const Quotient::RoomEvent *event);

    /**
     * @brief Output a string for the event to be used as  a RoomList subtitle.
     *
     * The output includes the username followed by the plain message, all with no
     * line breaks.
     */
    static QString subtitleText(const NeoChatRoom *room, const Quotient::RoomEvent *event);

    /**
     * @brief Return the media info for the event.
     *
     * An empty QVariantMap will be returned for any event that doesn't have any
     * media info.
     *
     * @return This should consist of the following:
     *  - source - The mxc URL for the media.
     *  - mimeType - The MIME type of the media (should be image/xxx for this delegate).
     *  - mimeIcon - The MIME icon name (should be image-xxx).
     *  - size - The file size in bytes.
     *  - width - The width in pixels of the audio media.
     *  - height - The height in pixels of the audio media.
     *  - tempInfo - mediaInfo (with the same properties as this except no tempInfo) for a temporary image while the file downloads.
     *  - isSticker - Whether the image is a sticker or not
     */
    static QVariantMap mediaInfo(const NeoChatRoom *room, const Quotient::RoomEvent *event);

    /**
     * @brief Whether the event is a reply to another in the timeline.
     *
     * @param showFallbacks whether message that have is_falling_back set true should
     *                      show the fallback reply. Leave true for non-threaded
     *                      timelines.
     */
    static bool hasReply(const Quotient::RoomEvent *event, bool showFallbacks = true);

    /**
     * @brief Return the Matrix ID of the event replied to.
     */
    static QString replyId(const Quotient::RoomEvent *event);

    /**
     * @brief Get the author of the event replied to in context of the room.
     *
     * An empty Quotient::RoomMember will be returned if the EventHandler hasn't had
     * the room or event initialised.
     *
     * @param isPending if the event is pending, i.e. has not been confirmed by
     *                  the server.
     *
     * @return a Quotient::RoomMember object for the user.
     *
     * @sa Quotient::RoomMember
     */
    static Quotient::RoomMember replyAuthor(const NeoChatRoom *room, const Quotient::RoomEvent *event);

    /**
     * @brief Whether the message is part of a thread.
     *
     * i.e. There is a rel_type of m.thread.
     */
    static bool isThreaded(const Quotient::RoomEvent *event);

    /**
     * @brief Return the Matrix ID of the thread's root message.
     *
     * Empty if this not part of a thread.
     */
    static QString threadRoot(const Quotient::RoomEvent *event);

    /**
     * @brief Return the latitude for the event.
     *
     * Returns -100.0 if the event doesn't have a location (latitudes are in the
     * range -90deg to +90deg so -100 is out of range).
     */
    static float latitude(const Quotient::RoomEvent *event);

    /**
     * @brief Return the longitude for the event.
     *
     * Returns -200.0 if the event doesn't have a location (latitudes are in the
     * range -180deg to +180deg so -200 is out of range).
     */
    static float longitude(const Quotient::RoomEvent *event);

    /**
     * @brief Return the type of location marker for the event.
     */
    static QString locationAssetType(const Quotient::RoomEvent *event);

private:
    static QString getBody(const NeoChatRoom *room, const Quotient::RoomEvent *event, Qt::TextFormat format, bool stripNewlines);
    static QString getMessageBody(const NeoChatRoom *room, const Quotient::RoomMessageEvent &event, Qt::TextFormat format, bool stripNewlines);

    static QVariantMap getMediaInfoForEvent(const NeoChatRoom *room, const Quotient::RoomEvent *event);
    QVariantMap static getMediaInfoFromFileInfo(const NeoChatRoom *room,
                                                const Quotient::EventContent::FileContentBase *fileContent,
                                                const QString &eventId,
                                                bool isThumbnail = false,
                                                bool isSticker = false);
    static QVariantMap getMediaInfoFromTumbnail(const NeoChatRoom *room, const Quotient::EventContent::Thumbnail &thumbnail, const QString &eventId);
};
