// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "permissionsmodel.h"
#include "powerlevel.h"

#include <Quotient/events/roompowerlevelsevent.h>

#include <KLazyLocalizedString>

static const QStringList defaultPermissions = {
    QStringLiteral("users_default"),
    QStringLiteral("state_default"),
    QStringLiteral("events_default"),
    QStringLiteral("invite"),
    QStringLiteral("kick"),
    QStringLiteral("ban"),
    QStringLiteral("redact"),
    QStringLiteral("m.reaction"),
    QStringLiteral("m.room.redaction"),
    QStringLiteral("m.room.power_levels"),
    QStringLiteral("m.room.name"),
    QStringLiteral("m.room.avatar"),
    QStringLiteral("m.room.canonical_alias"),
    QStringLiteral("m.room.topic"),
    QStringLiteral("m.room.encryption"),
    QStringLiteral("m.room.history_visibility"),
    QStringLiteral("m.room.pinned_events"),
    QStringLiteral("m.room.tombstone"),
    QStringLiteral("m.room.server_acl"),
    QStringLiteral("m.space.child"),
    QStringLiteral("m.space.parent"),
};

// Alternate name text for default permissions.
static const QHash<QString, KLazyLocalizedString> defaultPermissionNames = {
    {QStringLiteral("users_default"), kli18nc("Room permission type", "Default user power level")},
    {QStringLiteral("state_default"), kli18nc("Room permission type", "Default power level to set the room state")},
    {QStringLiteral("events_default"), kli18nc("Room permission type", "Default power level to send messages")},
    {QStringLiteral("invite"), kli18nc("Room permission type", "Invite users")},
    {QStringLiteral("kick"), kli18nc("Room permission type", "Kick users")},
    {QStringLiteral("ban"), kli18nc("Room permission type", "Ban users")},
    {QStringLiteral("redact"), kli18nc("Room permission type", "Remove messages sent by other users")},
    {QStringLiteral("m.reaction"), kli18nc("Room permission type", "Send reactions")},
    {QStringLiteral("m.room.redaction"), kli18nc("Room permission type", "Remove their own messages")},
    {QStringLiteral("m.room.power_levels"), kli18nc("Room permission type", "Change user permissions")},
    {QStringLiteral("m.room.name"), kli18nc("Room permission type", "Change the room name")},
    {QStringLiteral("m.room.avatar"), kli18nc("Room permission type", "Change the room avatar")},
    {QStringLiteral("m.room.canonical_alias"), kli18nc("Room permission type", "Change the room canonical alias")},
    {QStringLiteral("m.room.topic"), kli18nc("Room permission type", "Change the room topic")},
    {QStringLiteral("m.room.encryption"), kli18nc("Room permission type", "Enable encryption for the room")},
    {QStringLiteral("m.room.history_visibility"), kli18nc("Room permission type", "Change the room history visibility")},
    {QStringLiteral("m.room.pinned_events"), kli18nc("Room permission type", "Set pinned events")},
    {QStringLiteral("m.room.tombstone"), kli18nc("Room permission type", "Upgrade the room")},
    {QStringLiteral("m.room.server_acl"), kli18nc("Room permission type", "Set the room server access control list (ACL)")},
    {QStringLiteral("m.space.child"), kli18nc("Room permission type", "Set the children of this space")},
    {QStringLiteral("m.space.parent"), kli18nc("Room permission type", "Set the parent space of this room")},
};

// Subtitles for the default values.
static const QHash<QString, KLazyLocalizedString> defaultSubtitles = {
    {QStringLiteral("users_default"), kli18nc("Room permission type", "This is the power level for all new users when joining the room")},
    {QStringLiteral("state_default"), kli18nc("Room permission type", "This is used for all state events that do not have their own entry here")},
    {QStringLiteral("events_default"), kli18nc("Room permission type", "This is used for all message events that do not have their own entry here")},
};

// Permissions that should use the event default.
static const QStringList eventPermissions = {
    QStringLiteral("m.room.message"),
    QStringLiteral("m.reaction"),
    QStringLiteral("m.room.redaction"),
};

PermissionsModel::PermissionsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

NeoChatRoom *PermissionsModel::room() const
{
    return m_room;
}

void PermissionsModel::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }
    m_room = room;
    Q_EMIT roomChanged();

    initializeModel();
}

void PermissionsModel::initializeModel()
{
    beginResetModel();

    m_permissions.clear();

    if (m_room == nullptr) {
        endResetModel();
        return;
    }

    const auto currentPowerLevelEvent = m_room->currentState().get<Quotient::RoomPowerLevelsEvent>();
    if (currentPowerLevelEvent == nullptr) {
        return;
    }

    m_permissions.append(defaultPermissions);

    for (const auto &event : currentPowerLevelEvent->events().keys()) {
        if (!m_permissions.contains(event)) {
            m_permissions += event;
        }
    }

    endResetModel();
}

QVariant PermissionsModel::data(const QModelIndex &index, int role) const
{
    if (m_room == nullptr || !index.isValid()) {
        return {};
    }
    if (index.row() >= rowCount()) {
        qDebug() << "PushRuleModel, something's wrong: index.row() >= m_rules.count()";
        return {};
    }

    const auto permission = m_permissions.value(index.row());
    if (role == NameRole) {
        if (defaultPermissionNames.keys().contains(permission)) {
            return defaultPermissionNames.value(permission).toString();
        }
        return permission;
    }
    if (role == SubtitleRole) {
        if (permission.startsWith(QLatin1String("m.")) && defaultPermissionNames.keys().contains(permission)) {
            return permission;
        }
        if (defaultSubtitles.contains(permission)) {
            return defaultSubtitles.value(permission).toString();
        }
        return QString();
    }
    if (role == TypeRole) {
        return permission;
    }
    if (role == LevelRole) {
        const auto level = powerLevel(permission);
        if (level.has_value()) {
            return *level;
        }
        return {};
    }
    if (role == LevelNameRole) {
        const auto level = powerLevel(permission);
        if (level.has_value()) {
            return i18nc("%1 is the name of the power level, e.g. admin and %2 is the value that represents.",
                         "%1 (%2)",
                         PowerLevel::nameForLevel(PowerLevel::levelForValue(*level)),
                         *level);
        }
        return QString();
    }
    if (role == IsDefaultValueRole) {
        return permission.contains(QLatin1String("default"));
    }
    if (role == IsBasicPermissionRole) {
        return permission == QStringLiteral("invite") || permission == QStringLiteral("kick") || permission == QStringLiteral("ban")
            || permission == QStringLiteral("redact");
    }
    return {};
}

int PermissionsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_permissions.count();
}

QHash<int, QByteArray> PermissionsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[NameRole] = "name";
    roles[SubtitleRole] = "subtitle";
    roles[TypeRole] = "type";
    roles[LevelRole] = "level";
    roles[LevelNameRole] = "levelName";
    roles[IsDefaultValueRole] = "isDefaultValue";
    roles[IsBasicPermissionRole] = "isBasicPermission";
    return roles;
}

std::optional<int> PermissionsModel::powerLevel(const QString &permission) const
{
    if (m_room == nullptr) {
        return std::nullopt;
    }

    if (const auto currentPowerLevelEvent = m_room->currentState().get<Quotient::RoomPowerLevelsEvent>()) {
        if (permission == QStringLiteral("ban")) {
            return currentPowerLevelEvent->ban();
        } else if (permission == QStringLiteral("kick")) {
            return currentPowerLevelEvent->kick();
        } else if (permission == QStringLiteral("invite")) {
            return currentPowerLevelEvent->invite();
        } else if (permission == QStringLiteral("redact")) {
            return currentPowerLevelEvent->redact();
        } else if (permission == QStringLiteral("users_default")) {
            return currentPowerLevelEvent->usersDefault();
        } else if (permission == QStringLiteral("state_default")) {
            return currentPowerLevelEvent->stateDefault();
        } else if (permission == QStringLiteral("events_default")) {
            return currentPowerLevelEvent->eventsDefault();
        } else if (eventPermissions.contains(permission)) {
            return currentPowerLevelEvent->powerLevelForEvent(permission);
        } else {
            return currentPowerLevelEvent->powerLevelForState(permission);
        }
    }
    return std::nullopt;
}

void PermissionsModel::setPowerLevel(const QString &permission, const int &newPowerLevel)
{
    if (m_room == nullptr) {
        return;
    }

    int clampPowerLevel = std::clamp(newPowerLevel, -1, 100);

    const auto currentPowerLevel = powerLevel(permission);
    if (!currentPowerLevel.has_value() || currentPowerLevel == clampPowerLevel) {
        return;
    }

    if (auto currentPowerLevelEvent = m_room->currentState().get<Quotient::RoomPowerLevelsEvent>()) {
        auto powerLevelContent = currentPowerLevelEvent->contentJson();
        if (powerLevelContent.contains(permission)) {
            powerLevelContent[permission] = clampPowerLevel;
        } else {
            auto eventPowerLevels = powerLevelContent[QLatin1String("events")].toObject();
            eventPowerLevels[permission] = clampPowerLevel;
            powerLevelContent[QLatin1String("events")] = eventPowerLevels;
        }

        m_room->setState<Quotient::RoomPowerLevelsEvent>(Quotient::fromJson<Quotient::PowerLevelsEventContent>(powerLevelContent));
    }
}

#include "moc_permissionsmodel.cpp"
