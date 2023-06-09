// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QPointer>

class NeoChatRoom;

/**
 * Note: This model uses the ImagePackRoles from ImageContentManager as roles.
 */
class RoomImagePacksModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit RoomImagePacksModel(QObject *parent = nullptr);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &index) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] NeoChatRoom *currentRoom() const;
    void setCurrentRoom(NeoChatRoom *currentRoom);

Q_SIGNALS:
    void currentRoomChanged();

private:
    QPointer<NeoChatRoom> m_currentRoom;
};
