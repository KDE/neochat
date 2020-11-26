/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef MESSAGEEVENTMODEL_H
#define MESSAGEEVENTMODEL_H

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
        ReadMarkerRole,
        SpecialMarksRole,
        LongOperationRole,
        AnnotationRole,
        UserMarkerRole,

        ReplyRole,

        ShowAuthorRole,
        ShowSectionRole,

        ReactionRole,

        // For debugging
        EventResolvedTypeRole,
    };
    Q_ENUM(EventRoles)

    enum BubbleShapes {
        NoShape = 0,
        BeginShape,
        MiddleShape,
        EndShape,
    };

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

private Q_SLOTS:
    int refreshEvent(const QString &eventId);
    void refreshRow(int row);

private:
    NeoChatRoom *m_currentRoom = nullptr;
    QString lastReadEventId;
    int rowBelowInserted = -1;
    bool movingEvent = false;

    [[nodiscard]] int timelineBaseIndex() const;
    [[nodiscard]] QDateTime makeMessageTimestamp(const Quotient::Room::rev_iter_t &baseIt) const;
    [[nodiscard]] static QString renderDate(const QDateTime &timestamp);

    void refreshLastUserEvents(int baseTimelineRow);
    void refreshEventRoles(int row, const QVector<int> &roles = {});
    int refreshEventRoles(const QString &eventId, const QVector<int> &roles = {});

Q_SIGNALS:
    void roomChanged();
};

#endif // MESSAGEEVENTMODEL_H
