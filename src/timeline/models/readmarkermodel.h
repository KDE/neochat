// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <qtmetamacros.h>

#include "neochatroom.h"

/**
 * @class ReadMarkerModel
 *
 * This class defines the model for visualising a list of reactions to an event.
 */
class ReadMarkerModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief Returns a string with the names of the read markers at the event.
     *
     * This is in the form "x users: name 1, name 2, ...".
     */
    Q_PROPERTY(QString readMarkersString READ readMarkersString NOTIFY reactionUpdated)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        DisplayNameRole = Qt::DisplayRole, /**< The display name of the member in the room. */
        AvatarUrlRole, /**< The avatar for the member in the room. */
        ColorRole, /**< The color for the member. */
        UserIdRole, /** The user ID for the member. */
    };

    explicit ReadMarkerModel(const QString &eventId, NeoChatRoom *room);

    QString readMarkersString();

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
    void reactionUpdated();

private:
    QPointer<NeoChatRoom> m_room;
    QString m_eventId;
    QList<QString> m_markerIds;
};
