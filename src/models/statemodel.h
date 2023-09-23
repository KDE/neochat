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
        StateKeyRole, /**< The state key of the state event. */
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
     *
     * This is used to avoid having the model hold all the JSON data. The JSON for
     * a single item is only ever shown, no need to access simultaneously.
     */
    Q_INVOKABLE QByteArray stateEventJson(const QModelIndex &index);

Q_SIGNALS:
    void roomChanged();

private:
    NeoChatRoom *m_room = nullptr;

    /**
     * @brief The room state events in a QList.
     *
     * This is done for performance, accessing all the data from the parent QHash
     * was slower.
     */
    QList<std::pair<QString, QString>> m_stateEvents;
};
