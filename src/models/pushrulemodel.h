// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/csapi/definitions/push_rule.h>

#include "enums/pushrule.h"
#include "neochatconnection.h"

/**
 * @class PushRuleModel
 *
 * This class defines the model for managing notification push rule keywords.
 */
class PushRuleModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The default state for any newly created keyword rule.
     */
    Q_PROPERTY(PushRuleAction::Action defaultState READ defaultState WRITE setDefaultState NOTIFY defaultStateChanged)

    /**
     * @brief The global notification state.
     *
     * If this rule is set to off all push notifications are disabled regardless
     * of other settings.
     */
    Q_PROPERTY(bool globalNotificationsEnabled READ globalNotificationsEnabled WRITE setGlobalNotificationsEnabled NOTIFY globalNotificationsEnabledChanged)

    /**
     * @brief Whether the global notification state has been retrieved from the server.
     *
     * @sa globalNotificationsEnabled, PushRuleAction::Action
     */
    Q_PROPERTY(bool globalNotificationsSet READ globalNotificationsSet NOTIFY globalNotificationsSetChanged)

    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

public:
    struct Rule {
        QString id;
        PushRuleKind::Kind kind;
        PushRuleAction::Action action;
        PushRuleSection::Section section;
        bool enabled;
        QString roomId;
    };

    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        NameRole = Qt::DisplayRole, /**< The push rule name. */
        IdRole, /**< The push rule ID. */
        KindRole, /**< The kind of notification rule; override, content, etc. */
        ActionRole, /**< The PushRuleAction for the rule. */
        HighlightableRole, /**< Whether the rule can have a highlight action. */
        DeletableRole, /**< Whether the rule can be deleted the rule. */
        SectionRole, /**< The section to sort into in the settings page. */
        RoomIdRole, /**< The room the rule applies to (blank if global). */
    };
    Q_ENUM(EventRoles)

    explicit PushRuleModel(QObject *parent = nullptr);

    [[nodiscard]] PushRuleAction::Action defaultState() const;
    void setDefaultState(PushRuleAction::Action defaultState);

    [[nodiscard]] bool globalNotificationsEnabled() const;
    void setGlobalNotificationsEnabled(bool enabled);

    [[nodiscard]] bool globalNotificationsSet() const;

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
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setPushRuleAction(const QString &id, PushRuleAction::Action action);

    /**
     * @brief Add a new keyword to the model.
     */
    Q_INVOKABLE void addKeyword(const QString &keyword, const QString &roomId = {});

    /**
     * @brief Remove a keyword from the model.
     */
    Q_INVOKABLE void removeKeyword(const QString &keyword);

    void setConnection(NeoChatConnection *connection);
    NeoChatConnection *connection() const;

Q_SIGNALS:
    void defaultStateChanged();
    void globalNotificationsEnabledChanged();
    void globalNotificationsSetChanged();
    void connectionChanged();

private Q_SLOTS:
    void updateNotificationRules(const QString &type);

private:
    PushRuleAction::Action m_defaultKeywordAction;
    QList<Rule> m_rules;
    NeoChatConnection *m_connection;

    void setRules(QList<Quotient::PushRule> rules, PushRuleKind::Kind kind);

    int getRuleIndex(const QString &ruleId) const;
    PushRuleSection::Section getSection(Quotient::PushRule rule);

    void setNotificationRuleEnabled(const QString &kind, const QString &ruleId, bool enabled);
    void setNotificationRuleActions(const QString &kind, const QString &ruleId, PushRuleAction::Action action);
    PushRuleAction::Action variantToAction(const QList<QVariant> &actions, bool enabled);
    QList<QVariant> actionToVariant(PushRuleAction::Action action, const QString &sound = QStringLiteral("default"));
};
Q_DECLARE_METATYPE(PushRuleModel *)
