// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "linkpreviewer.h"
#include "messagecontentmodel.h"
#include "neochatroom.h"
#include "neochatroommember.h"
#include "pollhandler.h"
#include "readmarkermodel.h"
#include "threadmodel.h"

class ReactionModel;

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
    QML_ELEMENT

    /**
     * @brief The current room that the model is getting its messages from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        DelegateTypeRole = Qt::UserRole + 1, /**< The delegate type of the message. */
        EventIdRole, /**< The matrix event ID of the event. */
        TimeRole, /**< The timestamp for when the event was sent (as a QDateTime). */
        SectionRole, /**< The date of the event as a string. */
        AuthorRole, /**< The author of the event. */
        HighlightRole, /**< Whether the event should be highlighted. */
        SpecialMarksRole, /**< Whether the event is hidden or not. */
        ProgressInfoRole, /**< Progress info when downloading files. */
        GenericDisplayRole, /**< A generic string based upon the message type. */
        MediaInfoRole, /**< The media info for the event. */

        ContentModelRole, /**< The MessageContentModel for the event. */

        IsThreadedRole, /**< Whether the message is in a thread. */
        ThreadRootRole, /**< The Matrix ID of the thread root message, if any . */

        ShowSectionRole, /**< Whether the section header should be shown. */

        ReadMarkersRole, /**< The first 5 other users at the event for read marker tracking. */
        ShowReadMarkersRole, /**< Whether there are any other user read markers to be shown. */
        ReactionRole, /**< List model for this event. */
        ShowReactionsRole, /**< Whether there are any reactions to be shown. */

        VerifiedRole, /**< Whether an encrypted message is sent in a verified session. */
        AuthorDisplayNameRole, /**< The displayname for the event's sender; for name change events, the old displayname. */
        IsRedactedRole, /**< Whether an event has been deleted. */
        IsPendingRole, /**< Whether an event is waiting to be accepted by the server. */
        IsEditableRole, /**< Whether the event can be edited by the user. */
        LastRole, // Keep this last
    };
    Q_ENUM(EventRoles)

    explicit MessageEventModel(QObject *parent = nullptr);

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

    Q_INVOKABLE ThreadModel *threadModelForRootId(const QString &threadRootId) const;

protected:
    bool event(QEvent *event) override;

private:
    QPointer<NeoChatRoom> m_currentRoom = nullptr;
    QString lastReadEventId;
    QPersistentModelIndex m_lastReadEventIndex;
    int rowBelowInserted = -1;
    bool resetting = false;
    bool movingEvent = false;

    std::map<QString, std::unique_ptr<NeochatRoomMember>> m_memberObjects;
    std::map<QString, std::unique_ptr<MessageContentModel>> m_contentModels;
    QMap<QString, QSharedPointer<ReadMarkerModel>> m_readMarkerModels;
    QMap<QString, QSharedPointer<ThreadModel>> m_threadModels;
    QMap<QString, QSharedPointer<ReactionModel>> m_reactionModels;

    [[nodiscard]] int timelineBaseIndex() const;
    [[nodiscard]] QDateTime makeMessageTimestamp(const Quotient::Room::rev_iter_t &baseIt) const;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    void fullEventRefresh(int row);
    void refreshLastUserEvents(int baseTimelineRow);
    void refreshEventRoles(int row, const QList<int> &roles = {});
    int refreshEventRoles(const QString &eventId, const QList<int> &roles = {});
    void moveReadMarker(const QString &toEventId);

    void createEventObjects(const Quotient::RoomEvent *event);
    // Hack to ensure that we don't call endInsertRows when we haven't called beginInsertRows
    bool m_initialized = false;

Q_SIGNALS:
    void roomChanged();
};
