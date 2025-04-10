// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QString>

#include <Quotient/csapi/rooms.h>

#include "messagemodel.h"
#include "neochatroommember.h"

namespace Quotient
{
class Connection;
}

class NeoChatRoom;

/**
 * @class PinnedMessageModel
 *
 * This class defines the model for visualising a room's pinned messages.
 */
class PinnedMessageModel : public MessageModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief Whether the model is currently loading.
     */
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    explicit PinnedMessageModel(QObject *parent = nullptr);

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    bool loading() const;

Q_SIGNALS:
    void loadingChanged();

protected:
    std::optional<std::reference_wrapper<const Quotient::RoomEvent>> getEventForIndex(QModelIndex index) const override;

private:
    void setLoading(bool loading);
    void fill();

    bool m_loading = false;

    std::vector<Quotient::event_ptr_tt<Quotient::RoomEvent>> m_pinnedEvents;
};
