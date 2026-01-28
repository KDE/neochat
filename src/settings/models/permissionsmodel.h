// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <optional>

#include "neochatroom.h"

/**
 * @class PermissionsModel
 *
 * This class defines the model for managing room permission levels.
 */
class PermissionsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The room to show the permissions for
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        NameRole = Qt::UserRole, /**< The permission name. */
        SubtitleRole, /**< The description of the permission. */
        TypeRole, /**< The base type of the permission, normally the event type id except for ban, kick, etc. */
        LevelRole, /**< The current power level for the permission. */
        LevelNameRole, /**< The current power level for the permission as a string. */
        IsDefaultValueRole, /**< Whether the permission is a default value, e.g. for users. */
        IsBasicPermissionRole, /**< Whether the permission is one of the basic ones, e.g. kick, ban, etc. */
        IsMessagePermissionRole, /** Permissions related to messaging. */
        IsGeneralPermissionRole, /** Permissions related to general room management. */
    };
    Q_ENUM(Roles)

    explicit PermissionsModel(QObject *parent = nullptr);

    NeoChatRoom *room() const;
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

    /**
     * @brief Return the power level required for the given permission.
     */
    std::optional<int> powerLevel(const QString &permission) const;

    /**
     * @brief Set the power level required for the given permission.
     */
    Q_INVOKABLE void setPowerLevel(const QString &permission, const int &newPowerLevel);

Q_SIGNALS:
    void roomChanged();

private:
    QPointer<NeoChatRoom> m_room;

    QStringList m_permissions;

    void initializeModel();
};
