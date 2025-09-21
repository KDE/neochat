// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "pushrulemodel.h"

#include <QDebug>

#include <Quotient/converters.h>
#include <Quotient/csapi/definitions/push_ruleset.h>
#include <Quotient/csapi/pushrules.h>
#include <Quotient/jobs/basejob.h>

#include <KLazyLocalizedString>

// Alternate name text for default rules.
static const QHash<QString, KLazyLocalizedString> defaultRuleNames = {
    {u".m.rule.master"_s, kli18nc("Notification type", "Enable notifications for this account")},
    {u".m.rule.room_one_to_one"_s, kli18nc("Notification type", "Messages in one-to-one chats")},
    {u".m.rule.encrypted_room_one_to_one"_s, kli18nc("Notification type", "Encrypted messages in one-to-one chats")},
    {u".m.rule.message"_s, kli18nc("Notification type", "Messages in group chats")},
    {u".m.rule.encrypted"_s, kli18nc("Notification type", "Messages in encrypted group chats")},
    {u".m.rule.tombstone"_s, kli18nc("Notification type", "Room upgrade messages")},
    {u".m.rule.contains_display_name"_s, kli18nc("Notification type", "Messages containing my display name")},
    {u".m.rule.is_user_mention"_s, kli18nc("Notification type", "Messages which mention my Matrix user ID")},
    {u".m.rule.is_room_mention"_s, kli18nc("Notification type", "Messages which mention a room")},
    {u".m.rule.contains_user_name"_s, kli18nc("Notification type", "Messages containing the local part of my Matrix ID")},
    {u".m.rule.roomnotif"_s, kli18nc("Notification type", "Whole room (@room) notifications")},
    {u".m.rule.invite_for_me"_s, kli18nc("Notification type", "Invites to a room")},
    {u".m.rule.call"_s, kli18nc("Notification type", "Call invitation")},
};

// Sections for default rules.
static const QHash<QString, PushRuleSection::Section> defaultSections = {
    {u".m.rule.master"_s, PushRuleSection::Master},
    {u".m.rule.room_one_to_one"_s, PushRuleSection::Room},
    {u".m.rule.encrypted_room_one_to_one"_s, PushRuleSection::Room},
    {u".m.rule.message"_s, PushRuleSection::Room},
    {u".m.rule.encrypted"_s, PushRuleSection::Room},
    {u".m.rule.tombstone"_s, PushRuleSection::Room},
    {u".m.rule.contains_display_name"_s, PushRuleSection::Mentions},
    {u".m.rule.is_user_mention"_s, PushRuleSection::Mentions},
    {u".m.rule.is_room_mention"_s, PushRuleSection::Mentions},
    {u".m.rule.contains_user_name"_s, PushRuleSection::Mentions},
    {u".m.rule.roomnotif"_s, PushRuleSection::Mentions},
    {u".m.rule.invite_for_me"_s, PushRuleSection::Invites},
    {u".m.rule.call"_s, PushRuleSection::Undefined}, // TODO: make invites when VOIP added.
    {u".m.rule.suppress_notices"_s, PushRuleSection::Undefined},
    {u".m.rule.member_event"_s, PushRuleSection::Undefined},
    {u".m.rule.reaction"_s, PushRuleSection::Undefined},
    {u".m.rule.room.server_acl"_s, PushRuleSection::Undefined},
    {u".im.vector.jitsi"_s, PushRuleSection::Undefined},
};

// Default rules that don't have a highlight option as it would lead to all messages
// in a room being highlighted.
static const QStringList noHighlight = {
    u".m.rule.room_one_to_one"_s,
    u".m.rule.encrypted_room_one_to_one"_s,
    u".m.rule.message"_s,
    u".m.rule.encrypted"_s,
};

PushRuleModel::PushRuleModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void PushRuleModel::updateNotificationRules(const QString &type)
{
    if (type != u"m.push_rules"_s) {
        return;
    }

    const QJsonObject ruleDataJson = m_connection->accountDataJson(u"m.push_rules"_s);
    const Quotient::PushRuleset ruleData = Quotient::fromJson<Quotient::PushRuleset>(ruleDataJson["global"_L1].toObject());

    beginResetModel();
    m_rules.clear();

    // Doing this 5 times because PushRuleset is a struct.
    setRules(ruleData.override, PushRuleKind::Override);
    setRules(ruleData.content, PushRuleKind::Content);
    setRules(ruleData.room, PushRuleKind::Room);
    setRules(ruleData.sender, PushRuleKind::Sender);
    setRules(ruleData.underride, PushRuleKind::Underride);

    Q_EMIT globalNotificationsEnabledChanged();
    Q_EMIT globalNotificationsSetChanged();

    endResetModel();
}

void PushRuleModel::setRules(QList<Quotient::PushRule> rules, PushRuleKind::Kind kind)
{
    for (const auto &rule : rules) {
        QString roomId;
        if (rule.conditions.size() > 0) {
            for (const auto &condition : std::as_const(rule.conditions)) {
                if (condition.key == u"room_id"_s) {
                    roomId = condition.pattern;
                }
            }
        }

        m_rules.append(Rule{
            rule.ruleId,
            kind,
            variantToAction(rule.actions, rule.enabled),
            getSection(rule),
            rule.enabled,
            roomId,
        });
    }
}

int PushRuleModel::getRuleIndex(const QString &ruleId) const
{
    for (auto i = 0; i < m_rules.count(); i++) {
        if (m_rules[i].id == ruleId) {
            return i;
        }
    }
    return -1;
}

PushRuleSection::Section PushRuleModel::getSection(Quotient::PushRule rule)
{
    auto ruleId = rule.ruleId;

    if (defaultSections.contains(ruleId)) {
        return defaultSections.value(ruleId);
    } else {
        if (rule.ruleId.startsWith(u'.')) {
            return PushRuleSection::Unknown;
        }
        /**
         * If the rule name resolves to a matrix id for a room that the user is part
         * of it shouldn't appear in the global list as it's overriding the global
         * state for that room.
         *
         * Rooms that the user hasn't joined shouldn't have a rule.
         */
        if (m_connection->room(ruleId) != nullptr) {
            return PushRuleSection::Undefined;
        }
        /**
         * If the rule name resolves to a matrix id for a user  it shouldn't appear
         * in the global list as it's a rule to block notifications from a user and
         * is handled elsewhere.
         */
        auto testUserId = ruleId;
        // Rules for user matrix IDs often don't have the @ on the beginning so add
        // if not there to avoid malformed ID.
        if (!testUserId.startsWith(u'@')) {
            testUserId.prepend(u'@');
        }
        if (testUserId.startsWith(u'@') && !Quotient::serverPart(testUserId).isEmpty() && m_connection->user(testUserId) != nullptr) {
            return PushRuleSection::Undefined;
        }
        // If the rule has push conditions and one is a room ID it is a room only keyword.
        if (!rule.conditions.isEmpty()) {
            for (const auto &condition : std::as_const(rule.conditions)) {
                if (condition.key == u"room_id"_s) {
                    return PushRuleSection::RoomKeywords;
                }
            }
        }
        return PushRuleSection::Keywords;
    }
}

bool PushRuleModel::globalNotificationsEnabled() const
{
    auto masterIndex = getRuleIndex(u".m.rule.master"_s);
    if (masterIndex > -1) {
        return !m_rules[masterIndex].enabled;
    }
    return false;
}

void PushRuleModel::setGlobalNotificationsEnabled(bool enabled)
{
    setNotificationRuleEnabled(u"override"_s, u".m.rule.master"_s, !enabled);
}

bool PushRuleModel::globalNotificationsSet() const
{
    return getRuleIndex(u".m.rule.master"_s) > -1;
}

QVariant PushRuleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= rowCount()) {
        qDebug() << "PushRuleModel, something's wrong: index.row() >= m_rules.count()";
        return {};
    }

    if (role == NameRole) {
        auto ruleId = m_rules.at(index.row()).id;
        if (defaultRuleNames.contains(ruleId)) {
            return defaultRuleNames.value(ruleId).toString();
        } else {
            return ruleId;
        }
    }
    if (role == IdRole) {
        return m_rules.at(index.row()).id;
    }
    if (role == KindRole) {
        return m_rules.at(index.row()).kind;
    }
    if (role == ActionRole) {
        return m_rules.at(index.row()).action;
    }
    if (role == HighlightableRole) {
        return !noHighlight.contains(m_rules.at(index.row()).id);
    }
    if (role == DeletableRole) {
        return !m_rules.at(index.row()).id.startsWith(u"."_s);
    }
    if (role == SectionRole) {
        return m_rules.at(index.row()).section;
    }
    if (role == RoomIdRole) {
        return m_rules.at(index.row()).roomId;
    }

    return {};
}

int PushRuleModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_rules.count();
}

QHash<int, QByteArray> PushRuleModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[NameRole] = "name";
    roles[IdRole] = "id";
    roles[KindRole] = "kind";
    roles[ActionRole] = "ruleAction";
    roles[HighlightableRole] = "highlightable";
    roles[DeletableRole] = "deletable";
    roles[SectionRole] = "section";
    roles[RoomIdRole] = "roomId";
    return roles;
}

void PushRuleModel::setPushRuleAction(const QString &id, PushRuleAction::Action action)
{
    int index = getRuleIndex(id);
    if (index == -1) {
        return;
    }

    auto rule = m_rules[index];

    // Override rules need to be disabled when off so that other rules can match the message if they apply.
    if (action == PushRuleAction::Off && rule.kind == PushRuleKind::Override) {
        setNotificationRuleEnabled(PushRuleKind::kindString(rule.kind), rule.id, false);
    } else if (rule.kind == PushRuleKind::Override) {
        setNotificationRuleEnabled(PushRuleKind::kindString(rule.kind), rule.id, true);
    }

    setNotificationRuleActions(PushRuleKind::kindString(rule.kind), rule.id, action);
}

void PushRuleModel::addKeyword(const QString &keyword, const QString &roomId)
{
    if (!m_connection) {
        return;
    }

    PushRuleKind::Kind kind = PushRuleKind::Content;
    const QList<QVariant> actions = actionToVariant(m_connection->keywordPushRuleDefault());
    QList<Quotient::PushCondition> pushConditions;
    if (!roomId.isEmpty()) {
        kind = PushRuleKind::Override;

        Quotient::PushCondition roomCondition;
        roomCondition.kind = u"event_match"_s;
        roomCondition.key = u"room_id"_s;
        roomCondition.pattern = roomId;
        pushConditions.append(roomCondition);

        Quotient::PushCondition keywordCondition;
        keywordCondition.kind = u"event_match"_s;
        keywordCondition.key = u"content.body"_s;
        keywordCondition.pattern = keyword;
        pushConditions.append(keywordCondition);
    }

    auto job = m_connection->callApi<Quotient::SetPushRuleJob>(PushRuleKind::kindString(kind),
                                                               keyword,
                                                               actions,
                                                               QString(),
                                                               QString(),
                                                               pushConditions,
                                                               roomId.isEmpty() ? keyword : QString());
    connect(job, &Quotient::BaseJob::failure, this, [job, keyword]() {
        qWarning() << "Unable to set push rule for keyword %1: "_L1.arg(keyword) << job->errorString();
    });
}

/**
 * The rule never being removed from the list by this function is intentional. When
 * the server is updated the new push rule account data will be synced and it will
 * be removed when the model is updated then.
 */
void PushRuleModel::removeKeyword(const QString &keyword)
{
    int index = getRuleIndex(keyword);
    if (index == -1) {
        return;
    }

    auto kind = PushRuleKind::kindString(m_rules[index].kind);
    auto job = m_connection->callApi<Quotient::DeletePushRuleJob>(kind, m_rules[index].id);
    connect(job, &Quotient::BaseJob::failure, this, [this, job, index]() {
        qWarning() << "Unable to remove push rule for keyword %1: "_L1.arg(m_rules[index].id) << job->errorString();
    });
}

void PushRuleModel::setNotificationRuleEnabled(const QString &kind, const QString &ruleId, bool enabled)
{
    auto job = m_connection->callApi<Quotient::IsPushRuleEnabledJob>(kind, ruleId);
    connect(job, &Quotient::BaseJob::success, this, [job, kind, ruleId, enabled, this]() {
        if (job->enabled() != enabled) {
            m_connection->callApi<Quotient::SetPushRuleEnabledJob>(kind, ruleId, enabled);
        }
    });
}

void PushRuleModel::setNotificationRuleActions(const QString &kind, const QString &ruleId, PushRuleAction::Action action)
{
    QList<QVariant> actions;
    if (ruleId == u".m.rule.call"_s) {
        actions = actionToVariant(action, u"ring"_s);
    } else {
        actions = actionToVariant(action);
    }

    m_connection->callApi<Quotient::SetPushRuleActionsJob>(kind, ruleId, actions);
}

PushRuleAction::Action PushRuleModel::variantToAction(const QList<QVariant> &actions, bool enabled)
{
    bool notify = false;
    bool isNoisy = false;
    bool highlightEnabled = false;
    for (const auto &i : actions) {
        auto actionString = i.toString();
        if (!actionString.isEmpty()) {
            if (actionString == u"notify"_s) {
                notify = true;
            }
            continue;
        }

        QJsonObject action = i.toJsonObject();
        if (action["set_tweak"_L1].toString() == u"sound"_s) {
            isNoisy = true;
        } else if (action["set_tweak"_L1].toString() == u"highlight"_s) {
            if (action["value"_L1].toString() != u"false"_s) {
                highlightEnabled = true;
            }
        }
    }

    if (!enabled) {
        return PushRuleAction::Off;
    }

    if (notify) {
        if (isNoisy && highlightEnabled) {
            return PushRuleAction::NoisyHighlight;
        } else if (isNoisy) {
            return PushRuleAction::Noisy;
        } else if (highlightEnabled) {
            return PushRuleAction::Highlight;
        } else {
            return PushRuleAction::On;
        }
    } else {
        return PushRuleAction::Off;
    }
}

QList<QVariant> PushRuleModel::actionToVariant(PushRuleAction::Action action, const QString &sound)
{
    // The caller should never try to set the state to unknown.
    // It exists only as a default state to diable the settings options until the actual state is retrieved from the server.
    if (action == PushRuleAction::Unknown) {
        Q_ASSERT(false);
        return QList<QVariant>();
    }

    QList<QVariant> actions;

    if (action != PushRuleAction::Off) {
        actions.append(u"notify"_s);
    } else {
        actions.append(u"dont_notify"_s);
    }
    if (action == PushRuleAction::Noisy || action == PushRuleAction::NoisyHighlight) {
        QJsonObject soundTweak;
        soundTweak.insert("set_tweak"_L1, u"sound"_s);
        soundTweak.insert("value"_L1, sound);
        actions.append(soundTweak);
    }
    if (action == PushRuleAction::Highlight || action == PushRuleAction::NoisyHighlight) {
        QJsonObject highlightTweak;
        highlightTweak.insert("set_tweak"_L1, u"highlight"_s);
        actions.append(highlightTweak);
    }

    return actions;
}

NeoChatConnection *PushRuleModel::connection() const
{
    return m_connection;
}

void PushRuleModel::setConnection(NeoChatConnection *connection)
{
    if (connection == m_connection) {
        return;
    }
    m_connection = connection;
    Q_EMIT connectionChanged();

    if (m_connection) {
        connect(m_connection, &NeoChatConnection::accountDataChanged, this, &PushRuleModel::updateNotificationRules);
        updateNotificationRules(u"m.push_rules"_s);
    }
}

#include "moc_pushrulemodel.cpp"
