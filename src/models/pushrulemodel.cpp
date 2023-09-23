// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "pushrulemodel.h"

#include <QDebug>

#include <Quotient/connection.h>
#include <Quotient/converters.h>
#include <Quotient/csapi/definitions/push_ruleset.h>
#include <Quotient/csapi/pushrules.h>
#include <Quotient/jobs/basejob.h>

#include "controller.h"
#include "neochatconfig.h"

// Alternate name text for default rules.
static const QHash<QString, QString> defaultRuleNames = {
    {QStringLiteral(".m.rule.master"), QStringLiteral("Enable notifications for this account")},
    {QStringLiteral(".m.rule.room_one_to_one"), QStringLiteral("Messages in one-to-one chats")},
    {QStringLiteral(".m.rule.encrypted_room_one_to_one"), QStringLiteral("Encrypted messages in one-to-one chats")},
    {QStringLiteral(".m.rule.message"), QStringLiteral("Messages in group chats")},
    {QStringLiteral(".m.rule.encrypted"), QStringLiteral("Messages in encrypted group chats")},
    {QStringLiteral(".m.rule.tombstone"), QStringLiteral("Room upgrade messages")},
    {QStringLiteral(".m.rule.contains_display_name"), QStringLiteral("Messages containing my display name")},
    {QStringLiteral(".m.rule.is_user_mention"), QStringLiteral("Messages which mention my Matrix user ID.")},
    {QStringLiteral(".m.rule.is_room_mention"), QStringLiteral("Messages which mention a room.")},
    {QStringLiteral(".m.rule.contains_user_name"), QStringLiteral("Messages containing the local part of my Matrix ID.")},
    {QStringLiteral(".m.rule.roomnotif"), QStringLiteral("Whole room (@room) notifications")},
    {QStringLiteral(".m.rule.invite_for_me"), QStringLiteral("Invites to a room")},
    {QStringLiteral(".m.rule.call"), QStringLiteral("Call invitation")},
};

// Sections for default rules.
static const QHash<QString, PushNotificationSection::Section> defaultSections = {
    {QStringLiteral(".m.rule.master"), PushNotificationSection::Master},
    {QStringLiteral(".m.rule.room_one_to_one"), PushNotificationSection::Room},
    {QStringLiteral(".m.rule.encrypted_room_one_to_one"), PushNotificationSection::Room},
    {QStringLiteral(".m.rule.message"), PushNotificationSection::Room},
    {QStringLiteral(".m.rule.encrypted"), PushNotificationSection::Room},
    {QStringLiteral(".m.rule.tombstone"), PushNotificationSection::Room},
    {QStringLiteral(".m.rule.contains_display_name"), PushNotificationSection::Mentions},
    {QStringLiteral(".m.rule.is_user_mention"), PushNotificationSection::Mentions},
    {QStringLiteral(".m.rule.is_room_mention"), PushNotificationSection::Mentions},
    {QStringLiteral(".m.rule.contains_user_name"), PushNotificationSection::Mentions},
    {QStringLiteral(".m.rule.roomnotif"), PushNotificationSection::Mentions},
    {QStringLiteral(".m.rule.invite_for_me"), PushNotificationSection::Invites},
    {QStringLiteral(".m.rule.call"), PushNotificationSection::Undefined}, // TODO: make invites when VOIP added.
    {QStringLiteral(".m.rule.suppress_notices"), PushNotificationSection::Undefined},
    {QStringLiteral(".m.rule.member_event"), PushNotificationSection::Undefined},
    {QStringLiteral(".m.rule.reaction"), PushNotificationSection::Undefined},
    {QStringLiteral(".m.rule.room.server_acl"), PushNotificationSection::Undefined},
    {QStringLiteral(".im.vector.jitsi"), PushNotificationSection::Undefined},
};

// Default rules that don't have a highlight option as it would lead to all messages
// in a room being highlighted.
static const QStringList noHighlight = {
    QStringLiteral(".m.rule.room_one_to_one"),
    QStringLiteral(".m.rule.encrypted_room_one_to_one"),
    QStringLiteral(".m.rule.message"),
    QStringLiteral(".m.rule.encrypted"),
};

PushRuleModel::PushRuleModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_defaultKeywordAction = static_cast<PushNotificationAction::Action>(NeoChatConfig::self()->keywordPushRuleDefault());

    if (Controller::instance().activeConnection()) {
        controllerConnectionChanged();
    }
    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, &PushRuleModel::controllerConnectionChanged);
}

void PushRuleModel::controllerConnectionChanged()
{
    if (Controller::instance().activeConnection()) {
        connect(Controller::instance().activeConnection(), &Quotient::Connection::accountDataChanged, this, &PushRuleModel::updateNotificationRules);
        updateNotificationRules(QStringLiteral("m.push_rules"));
    }
}

void PushRuleModel::updateNotificationRules(const QString &type)
{
    if (type != QStringLiteral("m.push_rules")) {
        return;
    }

    const QJsonObject ruleDataJson = Controller::instance().activeConnection()->accountDataJson(QStringLiteral("m.push_rules"));
    const Quotient::PushRuleset ruleData = Quotient::fromJson<Quotient::PushRuleset>(ruleDataJson[QStringLiteral("global")].toObject());

    beginResetModel();
    m_rules.clear();

    // Doing this 5 times because PushRuleset is a struct.
    setRules(ruleData.override, PushNotificationKind::Override);
    setRules(ruleData.content, PushNotificationKind::Content);
    setRules(ruleData.room, PushNotificationKind::Room);
    setRules(ruleData.sender, PushNotificationKind::Sender);
    setRules(ruleData.underride, PushNotificationKind::Underride);

    Q_EMIT globalNotificationsEnabledChanged();
    Q_EMIT globalNotificationsSetChanged();

    endResetModel();
}

void PushRuleModel::setRules(QVector<Quotient::PushRule> rules, PushNotificationKind::Kind kind)
{
    for (const auto &rule : rules) {
        QString roomId;
        if (rule.conditions.size() > 0) {
            for (const auto &condition : rule.conditions) {
                if (condition.key == QStringLiteral("room_id")) {
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

PushNotificationSection::Section PushRuleModel::getSection(Quotient::PushRule rule)
{
    auto ruleId = rule.ruleId;

    if (defaultSections.contains(ruleId)) {
        return defaultSections.value(ruleId);
    } else {
        if (rule.ruleId.startsWith(u'.')) {
            return PushNotificationSection::Unknown;
        }
        /**
         * If the rule name resolves to a matrix id for a room that the user is part
         * of it shouldn't appear in the global list as it's overriding the global
         * state for that room.
         *
         * Rooms that the user hasn't joined shouldn't have a rule.
         */
        auto connection = Controller::instance().activeConnection();
        if (connection->room(ruleId) != nullptr) {
            return PushNotificationSection::Undefined;
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
        if (testUserId.startsWith(u'@') && !Quotient::serverPart(testUserId).isEmpty() && connection->user(testUserId) != nullptr) {
            return PushNotificationSection::Undefined;
        }
        // If the rule has push conditions and one is a room ID it is a room only keyword.
        if (!rule.conditions.isEmpty()) {
            for (auto condition : rule.conditions) {
                if (condition.key == QStringLiteral("room_id")) {
                    return PushNotificationSection::RoomKeywords;
                }
            }
        }
        return PushNotificationSection::Keywords;
    }
}

PushNotificationAction::Action PushRuleModel::defaultState() const
{
    return m_defaultKeywordAction;
}

void PushRuleModel::setDefaultState(PushNotificationAction::Action defaultState)
{
    if (defaultState == m_defaultKeywordAction) {
        return;
    }
    m_defaultKeywordAction = defaultState;
    NeoChatConfig::setKeywordPushRuleDefault(m_defaultKeywordAction);
    Q_EMIT defaultStateChanged();
}

bool PushRuleModel::globalNotificationsEnabled() const
{
    auto masterIndex = getRuleIndex(QStringLiteral(".m.rule.master"));
    if (masterIndex > -1) {
        return !m_rules[masterIndex].enabled;
    }
    return false;
}

void PushRuleModel::setGlobalNotificationsEnabled(bool enabled)
{
    setNotificationRuleEnabled(QStringLiteral("override"), QStringLiteral(".m.rule.master"), !enabled);
}

bool PushRuleModel::globalNotificationsSet() const
{
    return getRuleIndex(QStringLiteral(".m.rule.master")) > -1;
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
            return defaultRuleNames.value(ruleId);
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
        return !m_rules.at(index.row()).id.startsWith(QStringLiteral("."));
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

void PushRuleModel::setPushRuleAction(const QString &id, PushNotificationAction::Action action)
{
    int index = getRuleIndex(id);
    if (index == -1) {
        return;
    }

    auto rule = m_rules[index];

    // Override rules need to be disabled when off so that other rules can match the message if they apply.
    if (action == PushNotificationAction::Off && rule.kind == PushNotificationKind::Override) {
        setNotificationRuleEnabled(PushNotificationKind::kindString(rule.kind), rule.id, false);
    } else if (rule.kind == PushNotificationKind::Override) {
        setNotificationRuleEnabled(PushNotificationKind::kindString(rule.kind), rule.id, true);
    }

    setNotificationRuleActions(PushNotificationKind::kindString(rule.kind), rule.id, action);
}

void PushRuleModel::addKeyword(const QString &keyword, const QString &roomId)
{
    PushNotificationKind::Kind kind = PushNotificationKind::Content;
    const QVector<QVariant> actions = actionToVariant(m_defaultKeywordAction);
    QVector<Quotient::PushCondition> pushConditions;
    if (!roomId.isEmpty()) {
        kind = PushNotificationKind::Override;

        Quotient::PushCondition roomCondition;
        roomCondition.kind = QStringLiteral("event_match");
        roomCondition.key = QStringLiteral("room_id");
        roomCondition.pattern = roomId;
        pushConditions.append(roomCondition);

        Quotient::PushCondition keywordCondition;
        keywordCondition.kind = QStringLiteral("event_match");
        keywordCondition.key = QStringLiteral("content.body");
        keywordCondition.pattern = keyword;
        pushConditions.append(keywordCondition);
    }

    auto job = Controller::instance().activeConnection()->callApi<Quotient::SetPushRuleJob>(QLatin1String("global"),
                                                                                            PushNotificationKind::kindString(kind),
                                                                                            keyword,
                                                                                            actions,
                                                                                            QString(),
                                                                                            QString(),
                                                                                            pushConditions,
                                                                                            roomId.isEmpty() ? keyword : QString());
    connect(job, &Quotient::BaseJob::failure, this, [job, keyword]() {
        qWarning() << QLatin1String("Unable to set push rule for keyword %1: ").arg(keyword) << job->errorString();
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

    auto kind = PushNotificationKind::kindString(m_rules[index].kind);
    auto job = Controller::instance().activeConnection()->callApi<Quotient::DeletePushRuleJob>(QStringLiteral("global"), kind, m_rules[index].id);
    connect(job, &Quotient::BaseJob::failure, this, [this, job, index]() {
        qWarning() << QLatin1String("Unable to remove push rule for keyword %1: ").arg(m_rules[index].id) << job->errorString();
    });
}

void PushRuleModel::setNotificationRuleEnabled(const QString &kind, const QString &ruleId, bool enabled)
{
    auto job = Controller::instance().activeConnection()->callApi<Quotient::IsPushRuleEnabledJob>(QStringLiteral("global"), kind, ruleId);
    connect(job, &Quotient::BaseJob::success, this, [job, kind, ruleId, enabled]() {
        if (job->enabled() != enabled) {
            Controller::instance().activeConnection()->callApi<Quotient::SetPushRuleEnabledJob>(QStringLiteral("global"), kind, ruleId, enabled);
        }
    });
}

void PushRuleModel::setNotificationRuleActions(const QString &kind, const QString &ruleId, PushNotificationAction::Action action)
{
    QVector<QVariant> actions;
    if (ruleId == QStringLiteral(".m.rule.call")) {
        actions = actionToVariant(action, QStringLiteral("ring"));
    } else {
        actions = actionToVariant(action);
    }

    Controller::instance().activeConnection()->callApi<Quotient::SetPushRuleActionsJob>(QStringLiteral("global"), kind, ruleId, actions);
}

PushNotificationAction::Action PushRuleModel::variantToAction(const QVector<QVariant> &actions, bool enabled)
{
    bool notify = false;
    bool isNoisy = false;
    bool highlightEnabled = false;
    for (const auto &i : actions) {
        auto actionString = i.toString();
        if (!actionString.isEmpty()) {
            if (actionString == QLatin1String("notify")) {
                notify = true;
            }
            continue;
        }

        QJsonObject action = i.toJsonObject();
        if (action[QStringLiteral("set_tweak")].toString() == QStringLiteral("sound")) {
            isNoisy = true;
        } else if (action[QStringLiteral("set_tweak")].toString() == QStringLiteral("highlight")) {
            if (action[QStringLiteral("value")].toString() != QStringLiteral("false")) {
                highlightEnabled = true;
            }
        }
    }

    if (!enabled) {
        return PushNotificationAction::Off;
    }

    if (notify) {
        if (isNoisy && highlightEnabled) {
            return PushNotificationAction::NoisyHighlight;
        } else if (isNoisy) {
            return PushNotificationAction::Noisy;
        } else if (highlightEnabled) {
            return PushNotificationAction::Highlight;
        } else {
            return PushNotificationAction::On;
        }
    } else {
        return PushNotificationAction::Off;
    }
}

QVector<QVariant> PushRuleModel::actionToVariant(PushNotificationAction::Action action, const QString &sound)
{
    // The caller should never try to set the state to unknown.
    // It exists only as a default state to diable the settings options until the actual state is retrieved from the server.
    if (action == PushNotificationAction::Unknown) {
        Q_ASSERT(false);
        return QVector<QVariant>();
    }

    QVector<QVariant> actions;

    if (action != PushNotificationAction::Off) {
        actions.append(QStringLiteral("notify"));
    } else {
        actions.append(QStringLiteral("dont_notify"));
    }
    if (action == PushNotificationAction::Noisy || action == PushNotificationAction::NoisyHighlight) {
        QJsonObject soundTweak;
        soundTweak.insert(QStringLiteral("set_tweak"), QStringLiteral("sound"));
        soundTweak.insert(QStringLiteral("value"), sound);
        actions.append(soundTweak);
    }
    if (action == PushNotificationAction::Highlight || action == PushNotificationAction::NoisyHighlight) {
        QJsonObject highlightTweak;
        highlightTweak.insert(QStringLiteral("set_tweak"), QStringLiteral("highlight"));
        actions.append(highlightTweak);
    }

    return actions;
}

#include "moc_pushrulemodel.cpp"
