// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <KFormat>
#include <QAbstractListModel>
#include <QQmlEngine>

#include "linkpreviewer.h"
#include "neochatroom.h"
#include "pollhandler.h"

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
        TimeStringRole, /**< The timestamp for when the event was sent as a string (in QLocale::ShortFormat). */
        SectionRole, /**< The date of the event as a string. */
        AuthorRole, /**< The author of the event. */
        HighlightRole, /**< Whether the event should be highlighted. */
        SpecialMarksRole, /**< Whether the event is hidden or not. */
        ProgressInfoRole, /**< Progress info when downloading files. */
        GenericDisplayRole, /**< A generic string based upon the message type. */
        MediaInfoRole, /**< The media info for the event. */

        ContentModelRole, /**< The MessageContentModel for the event. */

        IsThreadedRole,
        ThreadRootRole,

        ShowSectionRole, /**< Whether the section header should be shown. */

        ReadMarkersRole, /**< The first 5 other users at the event for read marker tracking. */
        ExcessReadMarkersRole, /**< The number of other users at the event after the first 5. */
        ReadMarkersStringRole, /**< String with the display name and mxID of the users at the event. */
        ShowReadMarkersRole, /**< Whether there are any other user read markers to be shown. */
        ReactionRole, /**< List model for this event. */
        ShowReactionsRole, /**< Whether there are any reactions to be shown. */

        VerifiedRole, /**< Whether an encrypted message is sent in a verified session. */
        AuthorDisplayNameRole, /**< The displayname for the event's sender; for name change events, the old displayname. */
        IsRedactedRole, /**< Whether an event has been deleted. */
        IsPendingRole, /**< Whether an event is waiting to be accepted by the server. */
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

protected:
    bool event(QEvent *event) override;

private Q_SLOTS:
    int refreshEvent(const QString &eventId);
    void refreshRow(int row);

private:
    NeoChatRoom *m_currentRoom = nullptr;
    QString lastReadEventId;
    QPersistentModelIndex m_lastReadEventIndex;
    int rowBelowInserted = -1;
    bool resetting = false;
    bool movingEvent = false;
    KFormat m_format;

    QMap<QString, QSharedPointer<ReactionModel>> m_reactionModels;

    [[nodiscard]] int timelineBaseIndex() const;
    [[nodiscard]] QDateTime makeMessageTimestamp(const Quotient::Room::rev_iter_t &baseIt) const;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    void refreshLastUserEvents(int baseTimelineRow);
    void refreshEventRoles(int row, const QList<int> &roles = {});
    int refreshEventRoles(const QString &eventId, const QList<int> &roles = {});
    void moveReadMarker(const QString &toEventId);

    void createEventObjects(const Quotient::RoomMessageEvent *event);
    // Hack to ensure that we don't call endInsertRows when we haven't called beginInsertRows
    bool m_initialized = false;

Q_SIGNALS:
    void roomChanged();
    void fancyEffectsReasonFound(const QString &fancyEffect);
};
