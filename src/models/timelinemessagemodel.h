// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "messagemodel.h"

class ReactionModel;

namespace Quotient
{
class RoomEvent;
}

/**
 * @class TimelineMessageModel
 *
 * This class defines the model for visualising the room timeline.
 *
 * This model covers all event types in the timeline with many of the roles being
 * specific to a subset of events. This means the user needs to understand which
 * roles will return useful information for a given event type.
 *
 * @sa NeoChatRoom
 */
class TimelineMessageModel : public MessageModel
{
    Q_OBJECT
    QML_ELEMENT

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

    explicit TimelineMessageModel(QObject *parent = nullptr);

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void connectNewRoom();

    std::optional<std::reference_wrapper<const Quotient::RoomEvent>> getEventForIndex(QModelIndex index) const override;

    int rowBelowInserted = -1;
    bool resetting = false;
    bool movingEvent = false;

    int timelineServerIndex() const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    void moveReadMarker(const QString &toEventId);

    // Hack to ensure that we don't call endInsertRows when we haven't called beginInsertRows
    bool m_initialized = false;
};
