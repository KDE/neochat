// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "permissionsmodel.h"

#include <Quotient/events/roompowerlevelsevent.h>

#include <KLazyLocalizedString>

#include "enums/powerlevel.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr auto UsersDefaultKey = "users_default"_L1;
constexpr auto StateDefaultKey = "state_default"_L1;
constexpr auto EventsDefaultKey = "events_default"_L1;

constexpr auto InviteKey = "invite"_L1;
constexpr auto KickKey = "kick"_L1;
constexpr auto BanKey = "ban"_L1;
constexpr auto RedactKey = "redact"_L1;

static const QStringList defaultPermissions = {
    UsersDefaultKey,
    StateDefaultKey,
    EventsDefaultKey,
};

static const QStringList basicPermissions = {
    InviteKey,
    KickKey,
    BanKey,
    RedactKey,
};

static const QStringList knownPermissions = {
    u"m.reaction"_s,
    u"m.room.redaction"_s,
    u"m.room.power_levels"_s,
    u"m.room.name"_s,
    u"m.room.avatar"_s,
    u"m.room.canonical_alias"_s,
    u"m.room.topic"_s,
    u"m.room.encryption"_s,
    u"m.room.history_visibility"_s,
    u"m.room.pinned_events"_s,
    u"m.room.tombstone"_s,
    u"m.room.server_acl"_s,
    u"m.space.child"_s,
    u"m.space.parent"_s,
    u"org.matrix.msc3672.beacon_info"_s,
    u"org.matrix.msc3381.poll.start"_s,
    u"org.matrix.msc3381.poll.response"_s,
    u"org.matrix.msc3381.poll.end"_s,
};

// Alternate name text for default permissions.
static const QHash<QString, KLazyLocalizedString> permissionNames = {
    {UsersDefaultKey, kli18nc("Room permission type", "Default power level")},
    {StateDefaultKey, kli18nc("Room permission type", "Default power level to change room state")},
    {EventsDefaultKey, kli18nc("Room permission type", "Default power level to send messages")},
    {InviteKey, kli18nc("Room permission type", "Invite users")},
    {KickKey, kli18nc("Room permission type", "Kick users")},
    {BanKey, kli18nc("Room permission type", "Ban users")},
    {RedactKey, kli18nc("Room permission type", "Remove messages sent by other users")},
    {u"m.reaction"_s, kli18nc("Room permission type", "Send reactions")},
    {u"m.room.redaction"_s, kli18nc("Room permission type", "Remove their own messages")},
    {u"m.room.power_levels"_s, kli18nc("Room permission type", "Change user permissions")},
    {u"m.room.name"_s, kli18nc("Room permission type", "Change the room name")},
    {u"m.room.avatar"_s, kli18nc("Room permission type", "Change the room avatar")},
    {u"m.room.canonical_alias"_s, kli18nc("Room permission type", "Change the room canonical alias")},
    {u"m.room.topic"_s, kli18nc("Room permission type", "Change the room topic")},
    {u"m.room.encryption"_s, kli18nc("Room permission type", "Enable encryption for the room")},
    {u"m.room.history_visibility"_s, kli18nc("Room permission type", "Change the room history visibility")},
    {u"m.room.pinned_events"_s, kli18nc("Room permission type", "Pin and unpin messages")},
    {u"m.room.tombstone"_s, kli18nc("Room permission type", "Upgrade the room")},
    {u"m.room.server_acl"_s, kli18nc("Room permission type", "Set the room server access control list (ACL)")},
    {u"m.space.child"_s, kli18nc("Room permission type", "Set the children of this space")},
    {u"m.space.parent"_s, kli18nc("Room permission type", "Set the parent space of this room")},
    {u"org.matrix.msc3672.beacon_info"_s, kli18nc("Room permission type", "Send live location updates")},
    {u"org.matrix.msc3381.poll.start"_s, kli18nc("Room permission type", "Start polls")},
    {u"org.matrix.msc3381.poll.response"_s, kli18nc("Room permission type", "Vote in polls")},
    {u"org.matrix.msc3381.poll.end"_s, kli18nc("Room permission type", "Close polls")},
};

// Subtitles for the default values.
static const QHash<QString, KLazyLocalizedString> permissionSubtitles = {
    {UsersDefaultKey, kli18nc("Room permission type", "This is the power level for all new users when joining the room.")},
    {StateDefaultKey, kli18nc("Room permission type", "This is used for all state-type events that do not have their own entry.")},
    {EventsDefaultKey, kli18nc("Room permission type", "This is used for all message-type events that do not have their own entry.")},
};

// Permissions that should use the message event default.
static const QStringList eventPermissions = {
    u"m.room.message"_s,
    u"m.reaction"_s,
    u"m.room.redaction"_s,
    u"org.matrix.msc3381.poll.start"_s,
    u"org.matrix.msc3381.poll.response"_s,
    u"org.matrix.msc3381.poll.end"_s,
};

// Permissions related to messaging.
static const QStringList messagingPermissions = {
    u"m.reaction"_s,
    u"m.room.redaction"_s,
    u"org.matrix.msc3672.beacon_info"_s,
    u"org.matrix.msc3381.poll.start"_s,
    u"org.matrix.msc3381.poll.response"_s,
    u"org.matrix.msc3381.poll.end"_s,
};

// Permissions related to general room management.
static const QStringList generalPermissions = {
    u"m.room.power_levels"_s,
    u"m.room.name"_s,
    u"m.room.avatar"_s,
    u"m.room.canonical_alias"_s,
    u"m.room.topic"_s,
    u"m.room.encryption"_s,
    u"m.room.history_visibility"_s,
    u"m.room.pinned_events"_s,
    u"m.room.tombstone"_s,
    u"m.room.server_acl"_s,
    u"m.space.child"_s,
    u"m.space.parent"_s,
};
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
    m_permissions.append(basicPermissions);
    m_permissions.append(knownPermissions);

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
        if (permissionNames.keys().contains(permission)) {
            return permissionNames.value(permission).toString();
        }
        return permission;
    }
    if (role == SubtitleRole) {
        if (knownPermissions.contains(permission) && permissionNames.keys().contains(permission)) {
            return permission;
        }
        if (permissionSubtitles.contains(permission)) {
            return permissionSubtitles.value(permission).toString();
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
        return defaultPermissions.contains(permission);
    }
    if (role == IsBasicPermissionRole) {
        return basicPermissions.contains(permission);
    }
    if (role == IsMessagePermissionRole) {
        return messagingPermissions.contains(permission);
    }
    if (role == IsGeneralPermissionRole) {
        return generalPermissions.contains(permission);
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
    roles[IsMessagePermissionRole] = "isMessagePermission";
    roles[IsGeneralPermissionRole] = "isGeneralPermission";
    return roles;
}

std::optional<int> PermissionsModel::powerLevel(const QString &permission) const
{
    if (m_room == nullptr) {
        return std::nullopt;
    }

    if (const auto currentPowerLevelEvent = m_room->currentState().get<Quotient::RoomPowerLevelsEvent>()) {
        if (permission == BanKey) {
            return currentPowerLevelEvent->ban();
        } else if (permission == KickKey) {
            return currentPowerLevelEvent->kick();
        } else if (permission == InviteKey) {
            return currentPowerLevelEvent->invite();
        } else if (permission == RedactKey) {
            return currentPowerLevelEvent->redact();
        } else if (permission == UsersDefaultKey) {
            return currentPowerLevelEvent->usersDefault();
        } else if (permission == StateDefaultKey) {
            return currentPowerLevelEvent->stateDefault();
        } else if (permission == EventsDefaultKey) {
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
            // Deal with the case where a default or basic permission is missing from the event content erroneously.
        } else if (defaultPermissions.contains(permission) || basicPermissions.contains(permission)) {
            powerLevelContent[permission] = clampPowerLevel;
        } else {
            auto eventPowerLevels = powerLevelContent["events"_L1].toObject();
            eventPowerLevels[permission] = clampPowerLevel;
            powerLevelContent["events"_L1] = eventPowerLevels;
        }

        m_room->setState<Quotient::RoomPowerLevelsEvent>(Quotient::fromJson<Quotient::PowerLevelsEventContent>(powerLevelContent));
    }
}

#include "moc_permissionsmodel.cpp"
