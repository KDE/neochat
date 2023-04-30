// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>

#include "neochatroom.h"

/**
 * @class MessageEventModel
 *
 * This class defines the model for visualising the room timeline.
 *
 * This model covers all event types in the timeline with many of the roles being
 * specific to a subset of events. This means the user needs to understand which
 * roles will return useful information for a given event type.
 *
 * @sa NeoChatRoom
 */
class MessageEventModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The current room that the model is getting its messages from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    /**
     * @brief The type of delegate that is needed for the event.
     *
     * @note While similar this is not the matrix event or message type. This is
     *       to tell a QML ListView what delegate to show for each event. So while
     *       similar to the spec it is not the same.
     */
    enum DelegateType {
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
        Other, /**< Anything that cannot be classified as another type. */
    };
    Q_ENUM(DelegateType);

    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        DelegateTypeRole = Qt::UserRole + 1, /**< The delegate type of the message. */
        PlainText, /**< Plain text representation of the message. */
        EventIdRole, /**< The matrix event ID of the event. */
        TimeRole, /**< The timestamp for when the event was sent. */
        SectionRole, /**< The date of the event as a string. */
        AuthorRole, /**< The author of the event. */
        ContentRole, /**< The full message content. */
        ContentTypeRole, /**< The content mime type. */
        HighlightRole, /**< Whether the event should be highlighted. */
        SpecialMarksRole, /**< Whether the event is hidden or not. */
        LongOperationRole, /**< Progress info when downloading files. */
        FormattedBodyRole, /**< The formatted body of a rich message. */
        GenericDisplayRole, /**< A generic string based upon the message type. */

        MimeTypeRole, /**< The mime type of the message's file or media. */
        FileMimetypeIcon, /**< The icon name for the mime type of a file. */

        IsReplyRole, /**< Is the message a reply to another event. */
        ReplyAuthor, /**< The author of the event that was replied to. */
        ReplyRole, /**< The content data of the message that was replied to. */
        ReplyIdRole, /**< The matrix ID of the message that was replied to. */

        ShowAuthorRole, /**< Whether the author's name should be shown. */
        ShowSectionRole, /**< Whether the section header should be shown. */

        ReadMarkersRole, /**< Other users at the event for read marker tracking. */
        ReadMarkersStringRole, /**< String with the display name and mxID of the users at the event. */
        ShowReadMarkersRole, /**< Whether there are any other user read markers to be shown. */
        ReactionRole, /**< List of reactions to this event. */
        SourceRole, /**< The full message source JSON. */
        MediaUrlRole, /**< The source URL for any media in the message. */

        // For debugging
        EventResolvedTypeRole, /**< The event type the message. */
        AuthorIdRole, /**< Matrix ID of the message author. */

        VerifiedRole, /**< Whether an encrypted message is sent in a verified session. */
        DisplayNameForInitialsRole, /**< Sender's displayname, always without the matrix id. */
        AuthorDisplayNameRole, /**< The displayname for the event's sender; for name change events, the old displayname. */
        IsRedactedRole, /**< Whether an event has been deleted. */
        IsPendingRole, /**< Whether an event is waiting to be accepted by the server. */
        LatitudeRole, /**< Latitude for a location event. */
        LongitudeRole, /**< Longitude for a location event. */
        AssetRole, /**< Type of location event, e.g. self pin of the user location. */
        LastRole, // Keep this last
    };
    Q_ENUM(EventRoles)

    explicit MessageEventModel(QObject *parent = nullptr);
    ~MessageEventModel() override;

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Get the row number of the given event ID in the model.
     */
    Q_INVOKABLE [[nodiscard]] int eventIdToRow(const QString &eventID) const;

    /**
     * @brief Get the last message sent by the local user.
     *
     * @note This checks a maximum of the previous 35 message for performance reasons.
     *
     * @return a QVariantMap for the event with the following parameters:
     *  - eventId - The event ID.
     *  - formattedBody - The message text formatted as Qt::RichText.
     *  - message - The message text formatted as Qt::PlainText.
     */
    Q_INVOKABLE [[nodiscard]] QVariant getLastLocalUserMessageEventId();

    /**
     * @brief Get the last message sent earlier than the given row.
     *
     * @note This checks a maximum of the previous 35 message for performance reasons.
     *
     * @return a QVariantMap for the event with the following parameters:
     *  - eventId - The event ID.
     *  - message - The message text formatted as Qt::PlainText.
     *  - sender_id - The matrix ID of the sender.
     *  - at - The QModelIndex of the message.
     */
    Q_INVOKABLE [[nodiscard]] QVariant getLatestMessageFromRow(const int startRow);

    /**
     * @brief Load the event that the item at the given index replied to.
     *
     * This is used to ensure that the reply data is available when the message that
     * was replied to is outside the currently loaded timeline.
     */
    Q_INVOKABLE void loadReply(const QModelIndex &index);

private Q_SLOTS:
    int refreshEvent(const QString &eventId);
    void refreshRow(int row);

private:
    NeoChatRoom *m_currentRoom = nullptr;
    QString lastReadEventId;
    QPersistentModelIndex m_lastReadEventIndex;
    int rowBelowInserted = -1;
    bool movingEvent = false;

    [[nodiscard]] int timelineBaseIndex() const;
    [[nodiscard]] QDateTime makeMessageTimestamp(const Quotient::Room::rev_iter_t &baseIt) const;
    [[nodiscard]] static QString renderDate(const QDateTime &timestamp);

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    void refreshLastUserEvents(int baseTimelineRow);
    void refreshEventRoles(int row, const QVector<int> &roles = {});
    int refreshEventRoles(const QString &eventId, const QVector<int> &roles = {});
    void moveReadMarker(const QString &toEventId);

    const Quotient::RoomEvent *getReplyForEvent(const Quotient::RoomEvent &event) const;

    std::vector<Quotient::event_ptr_tt<Quotient::RoomEvent>> m_extraEvents;

Q_SIGNALS:
    void roomChanged();
    void fancyEffectsReasonFound(const QString &fancyEffect);
};
