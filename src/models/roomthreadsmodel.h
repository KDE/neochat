// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/events/roomevent.h>
#include <Quotient/events/roommessageevent.h>

#include "neochatroom.h"
#include "threadmodel.h"

/**
 * @class RoomThreadsModel
 *
 * This class defines the model for visualising a thread.
 */
class RoomThreadsModel : public QAbstractListModel
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
     *
     * For documentation of the roles, see MessageEventModel.
     *
     * Some of the roles exist only for compatibility with the MessageEventModel,
     * since the same delegates are used.
     */
    enum Roles {
        DisplayRole = Qt::DisplayRole,
        DelegateTypeRole,
        ShowAuthorRole,
        AuthorRole,
        ShowSectionRole,
        SectionRole,
        TimeRole,
        TimeStringRole,
        EventIdRole,
        ExcessReadMarkersRole,
        HighlightRole,
        ReadMarkersString,
        PlainTextRole,
        VerifiedRole,
        ProgressInfoRole,
        ShowReactionsRole,
        IsReplyRole,
        ReplyAuthorRole,
        ReplyIdRole,
        ReplyDelegateTypeRole,
        ReplyDisplayRole,
        ReplyMediaInfoRole,
        ReactionRole,
        ReadMarkersRole,
        IsPendingRole,
        ShowReadMarkersRole,
        MimeTypeRole,
        ShowLinkPreviewRole,
        LinkPreviewRole,
    };
    Q_ENUM(Roles)

    explicit RoomThreadsModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void roomChanged();

private:
    NeoChatRoom *m_room;
    Quotient::RoomEvents m_threads;
    QHash<QString, ThreadModel *> m_threadModels;

    void initializeModel();
};
