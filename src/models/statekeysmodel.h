// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "neochatroom.h"

/**
 * @class StateKeysModel
 *
 * This class defines the model for visualising the state keys for a certain type in a room.
 */
class StateKeysModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current room that the model is getting its state events from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged REQUIRED)

    /**
     * @brief The event type to list the stateKeys for
     */
    Q_PROPERTY(QString eventType READ eventType WRITE setEventType NOTIFY eventTypeChanged REQUIRED)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        StateKeyRole, /**< The state key of the state event. */
    };
    Q_ENUM(Roles)

    explicit StateKeysModel(QObject *parent = nullptr);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    QString eventType() const;
    void setEventType(const QString &eventType);

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
    void eventTypeChanged();

private:
    QPointer<NeoChatRoom> m_room;
    QString m_eventType;
    QVector<const Quotient::StateEvent *> m_stateKeys;
    void loadState();
};
