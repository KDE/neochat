// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QString>

#include <Quotient/csapi/rooms.h>

#include "neochatroommember.h"

namespace Quotient
{
class Connection;
}

class NeoChatRoom;

/**
 * @class PinnedMessagesModel
 *
 * This class defines the model for visualising a room's pinned messages.
 */
class PinnedMessagesModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current room to fetch the pinned messages from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief Whether the model is currently loading.
     */
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

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
        DelegateTypeRole = Qt::DisplayRole + 1,
        AuthorRole,
        ShowSectionRole,
        SectionRole,
        EventIdRole,
        ExcessReadMarkersRole,
        HighlightRole,
        ReadMarkersString,
        VerifiedRole,
        ShowReactionsRole,
        ReactionRole,
        ReadMarkersRole,
        IsPendingRole,
        ShowReadMarkersRole,
        IsThreadedRole,
        ThreadRootRole,
        ContentModelRole,
        IsEditableRole,
    };
    Q_ENUM(Roles)
    explicit PinnedMessagesModel(QObject *parent = nullptr);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    bool loading() const;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void roomChanged();
    void loadingChanged();

protected:
    bool event(QEvent *event) override;

private:
    void setLoading(bool loading);

    QPointer<NeoChatRoom> m_room;
    bool m_loading = false;

    std::vector<Quotient::event_ptr_tt<Quotient::RoomEvent>> m_pinnedEvents;
};
