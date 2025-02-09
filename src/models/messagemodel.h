// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <functional>

#include "neochatroom.h"
#include "pollhandler.h"
#include "readmarkermodel.h"
#include "threadmodel.h"

class ReactionModel;

/**
 * @class MessageModel
 *
 * This class defines a base model for visualising the room events.
 *
 * On its own MessageModel will result in an empty timeline as there is no mechanism
 * to retrieve events. This is by design as it allows the model to be inherited from
 * and the user can specify their own source of events, e.g. a room timeline or a
 * search result.
 *
 * The inherited model MUST do the following:
 *  - Define methods for retrieving events
 *  - Call newEventAdded() for each new event in the model so that all the required
 *    event objects are created.
 *  - Override getEventForIndex() so that the data() function can get an event for a
 *    given index
 *  - Override rowCount()
 *
 * Optionally the new model can:
 *  - override timelineServerIndex() if dealing with pending events otherwise the default
 *    function returns 0 which is correct for other use cases.
 *  - m_lastReadEventIndex is available to track a read marker location (so that the
 *    data function can output the appropriate values). The new class must implement
 *    the functionality to add, move, remove, etc though.
 *
 * @sa NeoChatRoom
 */
class MessageModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

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

        VerifiedRole, /**< Whether an encrypted message is sent in a verified session. */
        AuthorDisplayNameRole, /**< The displayname for the event's sender; for name change events, the old displayname. */
        IsRedactedRole, /**< Whether an event has been deleted. */
        IsPendingRole, /**< Whether an event is waiting to be accepted by the server. */
        IsEditableRole, /**< Whether the event can be edited by the user. */
        ShowAuthorRole, /**< Whether the author of a message should be shown. */
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

    /**
     * @brief Get a ThreadModel for the give thread root Matrix ID.
     */
    Q_INVOKABLE ThreadModel *threadModelForRootId(const QString &threadRootId) const;

Q_SIGNALS:
    /**
     * @brief Emitted when the room is changed.
     */
    void roomChanged();

    /**
     * @brief A signal to tell the MessageModel that a new event has been added.
     *
     * Any model inheriting from MessageModel needs to emit this signal for every
     * new event it adds.
     */
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

    QMap<QString, QSharedPointer<ReadMarkerModel>> m_readMarkerModels;

    void createEventObjects(const Quotient::RoomEvent *event, bool isPending = false);
};
