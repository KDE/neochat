// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>

#include "neochatroom.h"
#include "room.h"

class MessageEventModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    enum EventRoles {
        EventTypeRole = Qt::UserRole + 1,
        MessageRole,
        EventIdRole,
        TimeRole,
        SectionRole,
        AuthorRole,
        ContentRole,
        ContentTypeRole,
        HighlightRole,
        SpecialMarksRole,
        LongOperationRole,
        AnnotationRole,
        UserMarkerRole,
        FormattedBodyRole,

        ReplyRole,

        ShowAuthorRole,
        ShowSectionRole,

        ReactionRole,

        IsEditedRole,

        // For debugging
        EventResolvedTypeRole,
    };
    Q_ENUM(EventRoles)

    explicit MessageEventModel(QObject *parent = nullptr);
    ~MessageEventModel() override;

    [[nodiscard]] NeoChatRoom *room() const
    {
        return m_currentRoom;
    }
    void setRoom(NeoChatRoom *room);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE [[nodiscard]] int eventIDToIndex(const QString &eventID) const;
    Q_INVOKABLE [[nodiscard]] QVariant getLastLocalUserMessageEventId();
    Q_INVOKABLE [[nodiscard]] QVariant getLatestMessageFromIndex(const int baseline);

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

Q_SIGNALS:
    void roomChanged();
    void fancyEffectsReasonFound(const QString &fancyEffect);
};
