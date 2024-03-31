// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

#include "models/roomtreemodel.h"

/**
 * @class SortFilterRoomTreeModel
 *
 * This model sorts and filters the room list.
 *
 * There are numerous room sort orders available:
 *  - Categories - sort rooms by their NeoChatRoomType and then by last activty within
 *                 each category.
 *  - LastActivity - sort rooms by the last active time in the room.
 *  - Alphabetical - sort the rooms alphabetically by room name.
 *
 * The model can be given a filter string that will only show rooms who's name includes
 * the text.
 *
 * The model can also be given an active space ID and will only show rooms within
 * that space.
 *
 * All space rooms and upgraded rooms will also be filtered out.
 */
class SortFilterRoomTreeModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The text to use to filter room names.
     */
    Q_PROPERTY(QString filterText READ filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

    /**
     * @brief Set the ID of the space to show rooms for.
     */
    Q_PROPERTY(QString activeSpaceId READ activeSpaceId WRITE setActiveSpaceId NOTIFY activeSpaceIdChanged)

    /**
     * @brief Whether only direct chats should be shown.
     */
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    enum RoomSortOrder {
        Alphabetical,
        Activity,
    };
    Q_ENUM(RoomSortOrder)

    enum Mode {
        Rooms,
        DirectChats,
        All,
    };
    Q_ENUM(Mode)

    explicit SortFilterRoomTreeModel(RoomTreeModel *sourceModel, QObject *parent = nullptr);

    void setRoomSortOrder(RoomSortOrder sortOrder);

    void setFilterText(const QString &text);
    [[nodiscard]] QString filterText() const;

    QString activeSpaceId() const;
    void setActiveSpaceId(const QString &spaceId);

    Mode mode() const;
    void setMode(Mode mode);

protected:
    /**
     * @brief Returns true if the value of source_left is less than source_right.
     *
     * @sa QSortFilterProxyModel::lessThan
     */
    [[nodiscard]] bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    /**
     * @brief Whether a row should be shown out or not.
     *
     * @sa QSortFilterProxyModel::filterAcceptsRow
     */
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
    void filterTextChanged();
    void activeSpaceIdChanged();
    void modeChanged();

private:
    RoomSortOrder m_sortOrder = Activity;
    Mode m_mode = All;
    QString m_filterText;
    QString m_activeSpaceId;

    bool roleCmp(const QVariant &left, const QVariant &right) const;
    bool prioritiesCmp(const QVector<RoomTreeModel::EventRoles> &priorities, const QModelIndex &left, const QModelIndex &right) const;
};
