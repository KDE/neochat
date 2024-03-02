// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "neochatroom.h"

/**
 * @class StateModel
 *
 * This class defines the model for visualising the state events in a room.
 */
class StateModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current room that the model is getting its state events from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        TypeRole = 0, /**< The type of the state event. */
        EventCountRole, /**< Number of events of this type. */
    };
    Q_ENUM(Roles)

    explicit StateModel(QObject *parent = nullptr);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Get the full JSON for an event.
     */
    Q_INVOKABLE QByteArray stateEventJson(const QModelIndex &index);

Q_SIGNALS:
    void roomChanged();

private:
    QPointer<NeoChatRoom> m_room;

    /**
     * @brief A map from state event type to number of events of that type
     */
    QMap<QString, QList<QString>> m_stateEvents;
    void loadState();
};
