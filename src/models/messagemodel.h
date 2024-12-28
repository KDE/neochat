// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <functional>

#include "messagecontentmodel.h"
#include "neochatroom.h"
#include "neochatroommember.h"
#include "pollhandler.h"
#include "readmarkermodel.h"
#include "threadmodel.h"

class ReactionModel;

/**
 * @class MessageModel
 *
 * This class defines a model for visualising the room events.
 *
 * This model covers all event types in the room with many of the roles being
 * specific to a subset of events. This means the user needs to understand which
 * roles will return useful information for a given event type.
 *
 * @sa NeoChatRoom
 */
class MessageModel : public QAbstractListModel
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

    explicit MessageModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

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

Q_SIGNALS:
    void roomChanged();
    void newEventAdded(const Quotient::RoomEvent *event, bool isPending = false);

protected:
    QPointer<NeoChatRoom> m_room;
    QPersistentModelIndex m_lastReadEventIndex;

    virtual int timelineServerIndex() const;
    virtual std::optional<std::reference_wrapper<const Quotient::RoomEvent>> getEventForIndex(QModelIndex index) const;

    void fullEventRefresh(int row);
    int refreshEventRoles(const QString &eventId, const QList<int> &roles = {});
    void refreshEventRoles(int row, const QList<int> &roles = {});
    void refreshLastUserEvents(int baseTimelineRow);

    void clearModel();
    void clearEventObjects();

    bool event(QEvent *event) override;

private:
    bool resetting = false;
    bool movingEvent = false;

    std::map<QString, std::unique_ptr<NeochatRoomMember>> m_memberObjects;
    std::map<QString, std::unique_ptr<MessageContentModel>> m_contentModels;
    QMap<QString, QSharedPointer<ReadMarkerModel>> m_readMarkerModels;
    QMap<QString, QSharedPointer<ThreadModel>> m_threadModels;
    QMap<QString, QSharedPointer<ReactionModel>> m_reactionModels;

    void createEventObjects(const Quotient::RoomEvent *event, bool isPending = false);
};
