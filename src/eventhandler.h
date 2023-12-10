// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <KFormat>

#include <Quotient/eventitem.h>
#include <Quotient/events/roomevent.h>
#include <Quotient/events/roommessageevent.h>

#include "enums/delegatetype.h"

class LinkPreviewer;
class NeoChatRoom;
class ReactionModel;

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
class EventHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Return the current room the EventHandler is using.
     */
    const NeoChatRoom *getRoom() const;

    /**
     * @brief Set the current room the EventHandler to using.
     */
    void setRoom(const NeoChatRoom *room);

    /**
     * @brief Return the current event the EventHandler is using.
     */
    const Quotient::Event *getEvent() const;

    /**
     * @brief Set the current event the EventHandler to using.
     */
    void setEvent(const Quotient::RoomEvent *event);

    /**
     * @brief Return the Matrix ID of the event.
     */
    QString getId() const;

    /**
     * @brief Return the DelegateType of the event.
     *
     * @note While similar this is not the matrix event or message type. This is
     *       to tell a QML ListView what delegate to show for each event. So while
     *       similar to the spec it is not the same.
     */
    DelegateType::Type getDelegateType() const;

    /**
     * @brief Get the author of the event in context of the room.
     *
     * This is different to getting a Quotient::User object
     * as neither of those can provide details like the displayName or avatarMediaId
     * without the room context as these can vary from room to room. This function
     * uses the room context and outputs the result as QVariantMap.
     *
     * An empty QVariantMap will be returned if the EventHandler hasn't had the room
     * intialised. An empty user (i.e. a QVariantMap with all the correct keys
     * but empty values) will be returned if the room has been set but not an event.
     *
     * @param isPending if the event is pending, i.e. has not been confirmed by
     *                  the server.
     *
     * @return a QVariantMap for the user with the following properties:
     *  - isLocalUser - Whether the user is the local user.
     *  - id - The matrix ID of the user.
     *  - displayName - Display name in the context of this room.
     *  - avatarSource - The mxc URL for the user's avatar in the current room.
     *  - avatarMediaId - Avatar id in the context of this room.
     *  - color - Color for the user.
     *  - object - The Quotient::User object for the user.
     *
     * @sa Quotient::User
     */
    QVariantMap getAuthor(bool isPending = false) const;

    /**
     * @brief Get the display name of the event author.
     *
     * This method is separate from getAuthor() and special in that it will return
     * the old display name of the author if the current event is one that caused it
     * to change. This allows for scenarios where the UI wishes to notify that a
     * user's display name has changed and what it changed from.
     *
     * @param isPending whether the event is pending as this cannot be derived from
     *                  just the event object.
     */
    QString getAuthorDisplayName(bool isPending = false) const;

    /**
     * @brief Get the display name of the event author but with any newlines removed.
     *
     * Turns out you can put newlines in your display name so we need to handle that
     * primarily for the room list subtitle.
     *
     * @param isPending whether the event is pending as this cannot be derived from
     *                  just the event object.
     */
    QString singleLineAuthorDisplayname(bool isPending = false) const;

    /**
     * @brief Return a QDateTime object for the event timestamp.
     */
    QDateTime getTime(bool isPending = false, QDateTime lastUpdated = {}) const;

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
    QString getTimeString(bool relative, QLocale::FormatType format = QLocale::ShortFormat, bool isPending = false, QDateTime lastUpdated = {}) const;

    /**
     * @brief Whether the event should be highlighted in the timeline.
     *
     * @note Messages in direct chats are never highlighted.
     */
    bool isHighlighted();

    /**
     * @brief Whether the event should be hidden in the timeline.
     *
     * This could be for numerous reasons, e.g. if it's a replacement event, if the
     * user has hidden all state events or if the sender has been ignored by the local
     * user.
     */
    bool isHidden();

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
    QString getRichBody(bool stripNewlines = false) const;

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
    QString getPlainBody(bool stripNewlines = false) const;

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
     * @sa getRichBody(), getPlainBody()
     */
    QString getGenericBody() const;

    /**
     * @brief Output a string for the event to be used as  a RoomList subtitle.
     *
     * The output includes the username followed by the plain message, all with no
     * line breaks.
     */
    QString subtitleText() const;

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
     */
    QVariantMap getMediaInfo() const;

    /**
     * @brief Return a LinkPreviewer object for the event.
     *
     * A nullptr will be returned for any event that doesn't have any links so the
     * return should be null checked and an empty LinkPreviewer provided if null.
     *
     * @sa LinkPreviewer
     */
    QSharedPointer<LinkPreviewer> getLinkPreviewer() const;

    /**
     * @brief Return a ReactionModel object for the event.
     *
     * A nullptr will be returned for any event that doesn't have any links so the
     * return should be null checked and an empty QVariantList (or other suitable
     * empty mode) provided if null.
     */
    QSharedPointer<ReactionModel> getReactions() const;

    /**
     * @brief Whether the event is a reply to another in the timeline.
     */
    bool hasReply() const;

    /**
     * @brief Return the Matrix ID of the event replied to.
     */
    QString getReplyId() const;

    /**
     * @brief Return the DelegateType of the event replied to.
     *
     * @note While similar this is not the matrix event or message type. This is
     *       to tell a QML ListView what delegate to show for each event. So while
     *       similar to the spec it is not the same.
     */
    DelegateType::Type getReplyDelegateType() const;

    /**
     * @brief Get the author of the event replied to in context of the room.
     *
     * This is different to getting a Quotient::User object
     * as neither of those can provide details like the displayName or avatarMediaId
     * without the room context as these can vary from room to room. This function
     * uses the room context and outputs the result as QVariantMap.
     *
     * An empty QVariantMap will be returned if the EventHandler hasn't had the room
     * intialised. An empty user (i.e. a QVariantMap with all the correct keys
     * but empty values) will be returned if the room has been set but not an event.
     *
     * @return a QVariantMap for the user with the following properties:
     *  - isLocalUser - Whether the user is the local user.
     *  - id - The matrix ID of the user.
     *  - displayName - Display name in the context of this room.
     *  - avatarSource - The mxc URL for the user's avatar in the current room.
     *  - avatarMediaId - Avatar id in the context of this room.
     *  - color - Color for the user.
     *  - object - The Quotient::User object for the user.
     *
     * @sa Quotient::User
     */
    QVariantMap getReplyAuthor() const;

    /**
     * @brief Output a string for the message content of the event replied to ready
     * for display in a rich text field.
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
    QString getReplyRichBody(bool stripNewlines = false) const;

    /**
     * @brief Output a string for the message content of the event replied to ready
     * for display in a plain text field.
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
    QString getReplyPlainBody(bool stripNewlines = false) const;

    /**
     * @brief Return the media info for the event replied to.
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
     */
    QVariantMap getReplyMediaInfo() const;

    /**
     * @brief Whether the message is part of a thread.
     *
     * i.e. There is a rel_type of m.thread.
     */
    bool isThreaded() const;

    /**
     * @brief Return the Matrix ID of the thread's root message.
     *
     * Empty if this not part of a thread.
     */
    QString threadRoot() const;

    /**
     * @brief Return the latitude for the event.
     *
     * Returns -100.0 if the event doesn't have a location (latitudes are in the
     * range -90deg to +90deg so -100 is out of range).
     */
    float getLatitude() const;

    /**
     * @brief Return the longitude for the event.
     *
     * Returns -200.0 if the event doesn't have a location (latitudes are in the
     * range -180deg to +180deg so -200 is out of range).
     */
    float getLongitude() const;

    /**
     * @brief Return the type of location marker for the event.
     */
    QString getLocationAssetType() const;

    /**
     * @brief Whether the event has any read marker for other users.
     */
    bool hasReadMarkers() const;

    /**
     * @brief Returns a list of user read marker for the event.
     *
     * @param maxMarkers the maximum number of users to return. Usually the number
     *                   of user read makers shown is limited to not clutter the UI.
     *                   This needs to be the same as used in getNumberExcessReadMarkers
     *                   so that the markers line up with the number displayed, i.e.
     *                   the number of users shown plus the excess number will be
     *                   the total number of other user read markers at an event.
     */
    QVariantList getReadMarkers(int maxMarkers = 5) const;

    /**
     * @brief Returns the number of excess user read markers for the event.
     *
     * This returns a string in the form "+ x" ready for use in the UI.
     *
     * @param maxMarkers the maximum number of markers shown in the UI. This needs to
     *                   be the same as used in getReadMarkers so that the value lines
     *                   up with the number displayed, i.e. the number of users shown
     *                   plus the excess number will be the total number of other user
     *                   read markers at an event.
     */
    QString getNumberExcessReadMarkers(int maxMarkers = 5) const;

    /**
     * @brief Returns a string with the names of the read markers at the event.
     *
     * This is in the form "x users: name 1, name 2, ...".
     */
    QString getReadMarkersString() const;

private:
    const NeoChatRoom *m_room = nullptr;
    const Quotient::RoomEvent *m_event = nullptr;

    KFormat m_format;

    DelegateType::Type getDelegateTypeForEvent(const Quotient::RoomEvent *event) const;

    QString getBody(const Quotient::RoomEvent *event, Qt::TextFormat format, bool stripNewlines) const;
    QString getMessageBody(const Quotient::RoomMessageEvent &event, Qt::TextFormat format, bool stripNewlines) const;

    QVariantMap getMediaInfoForEvent(const Quotient::RoomEvent *event) const;
    QVariantMap getMediaInfoFromFileInfo(const Quotient::EventContent::FileInfo *fileInfo, const QString &eventId, bool isThumbnail = false) const;
};
