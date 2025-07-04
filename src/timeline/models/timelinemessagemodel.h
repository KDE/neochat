// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "messagemodel.h"

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
    explicit TimelineMessageModel(QObject *parent = nullptr);

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

private:
    void connectNewRoom();

    std::optional<std::reference_wrapper<const Quotient::RoomEvent>> getEventForIndex(QModelIndex index) const override;

    int rowBelowInserted = -1;

    int timelineServerIndex() const override;

    // Hack to ensure that we don't call endInsertRows when we haven't called beginInsertRows
    bool m_initialized = false;
};
