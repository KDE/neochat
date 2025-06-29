// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <functional>

#include "neochatroom.h"
#include "readmarkermodel.h"

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

    /**
     * @brief The model index of the read marker.
     */
    Q_PROPERTY(QPersistentModelIndex readMarkerIndex READ readMarkerIndex NOTIFY readMarkerIndexChanged)

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
        IsPollRole, /**< Whether the message is a poll. */

        ShowSectionRole, /**< Whether the section header should be shown. */

        ReadMarkersRole, /**< The first 5 other users at the event for read marker tracking. */
        ShowReadMarkersRole, /**< Whether there are any other user read markers to be shown. */

        VerifiedRole, /**< Whether an encrypted message is sent in a verified session. */
        AuthorDisplayNameRole, /**< The displayname for the event's sender; for name change events, the old displayname. */
        IsRedactedRole, /**< Whether an event has been deleted. */
        IsPendingRole, /**< Whether an event is waiting to be accepted by the server. */
        IsEditableRole, /**< Whether the event can be edited by the user. */
        ShowAuthorRole, /**< Whether the author of a message should be shown. */
        EventTypeRole, /**< The matrix event type of this message. */
        LastRole, // Keep this last
    };
    Q_ENUM(EventRoles)

    explicit MessageModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    QPersistentModelIndex readMarkerIndex() const;

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
     * @brief Get the QModelIndex of the given event ID in the model.
     */
    Q_INVOKABLE QModelIndex indexforEventId(const QString &eventId) const;

    static void setHiddenFilter(std::function<bool(const Quotient::RoomEvent *)> hiddenFilter);

    static void setThreadsEnabled(bool enableThreads);

Q_SIGNALS:
    /**
     * @brief Emitted when the room is changed.
     */
    void roomChanged();

    /**
     * @brief Emitted when the reader marker is added.
     */
    void readMarkerAdded();

    /**
     * @brief Emitted when the reader marker index is changed.
     */
    void readMarkerIndexChanged();

    /**
     * @brief Emitted when the model is about to reset.
     */
    void modelAboutToBeReset();

    /**
     * @brief Emitted when the model has been reset.
     */
    void modelResetComplete();

    /**
     * @brief A signal to tell the MessageModel that a new event has been added.
     *
     * Any model inheriting from MessageModel needs to emit this signal for every
     * new event it adds.
     */
    void newEventAdded(const Quotient::RoomEvent *event);

    /**
     * @brief A signal that should be emitted when the local user posts a new event in the room.
     */
    void newLocalUserEventAdded();

    void threadsEnabledChanged();

protected:
    QPointer<NeoChatRoom> m_room;
    QPersistentModelIndex m_lastReadEventIndex;

    virtual int timelineServerIndex() const;
    virtual std::optional<std::reference_wrapper<const Quotient::RoomEvent>> getEventForIndex(QModelIndex index) const;

    void fullEventRefresh(int row);
    int refreshEventRoles(const QString &eventId, const QList<int> &roles = {});
    void refreshEventRoles(int row, const QList<int> &roles = {});
    void refreshLastUserEvents(int baseTimelineRow);

    void moveReadMarker(const QString &toEventId);

    void clearModel();
    void clearEventObjects();

    bool event(QEvent *event) override;

private:
    bool resetting = false;
    bool movingEvent = false;

    QMap<QString, QSharedPointer<ReadMarkerModel>> m_readMarkerModels;

    void createEventObjects(const Quotient::RoomEvent *event);

    static std::function<bool(const Quotient::RoomEvent *)> m_hiddenFilter;
    static bool m_threadsEnabled;
};
