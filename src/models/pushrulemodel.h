// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>

#include <csapi/definitions/push_rule.h>

#include "notificationsmanager.h"

/**
 * @class PushNotificationKind
 *
 * A class with the Kind enum for push notifications and helper functions.
 *
 * The kind relates to the kinds of push rule definied in the matrix spec, see
 * https://spec.matrix.org/v1.7/client-server-api/#push-rules for full details.
 */
class PushNotificationKind : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Defines the different kinds of push rule.
     */
    enum Kind {
        Override = 0, /**< The highest priority rules. */
        Content, /**< These configure behaviour for messages that match certain patterns. */
        Room, /**< These rules change the behaviour of all messages for a given room. */
        Sender, /**< These rules configure notification behaviour for messages from a specific Matrix user ID. */
        Underride, /**< These are identical to override rules, but have a lower priority than content, room and sender rules. */
    };
    Q_ENUM(Kind)

    /**
     * @brief Translate the Kind enum value to a human readable string.
     *
     * @sa Kind
     */
    static QString kindString(Kind kind)
    {
        switch (kind) {
        case Kind::Override:
            return QLatin1String("override");
        case Kind::Content:
            return QLatin1String("content");
        case Kind::Room:
            return QLatin1String("room");
        case Kind::Sender:
            return QLatin1String("sender");
        case Kind::Underride:
            return QLatin1String("underride");
        default:
            return {};
        }
    };
};

/**
 * @class PushNotificationSection
 *
 * A class with the Section enum for push notifications and helper functions.
 *
 * @note This is different from the PushNotificationKind and instead is used for sorting
 *       in the settings page which is not necessarily by Kind.
 *
 * @sa PushNotificationKind
 */
class PushNotificationSection : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Defines the sections to sort push rules into.
     */
    enum Section {
        Master = 0, /**< The master push rule */
        Room, /**< Push rules relating to all rooms. */
        Mentions, /**< Push rules relating to user mentions. */
        Keywords, /**< Global Keyword push rules. */
        RoomKeywords, /**< Keyword push rules that only apply to a specific room. */
        Invites, /**< Push rules relating to invites. */
        /**
         * @brief Push rules that should never be shown.
         *
         * There are numerous rules that get set that shouldn't be shown in the general
         * list e.g. The array of rules used to override global settings in individual
         * rooms.
         */
        Undefined,
    };
    Q_ENUM(Section)

    /**
     * @brief Translate the Section enum value to a human readable string.
     *
     * @sa Section
     */
    static QString sectionString(Section section)
    {
        switch (section) {
        case Section::Master:
            return QLatin1String("Master");
        case Section::Room:
            return QLatin1String("Room Notifications");
        case Section::Mentions:
            return QLatin1String("@Mentions");
        case Section::Keywords:
            return QLatin1String("Keywords");
        case Section::Invites:
            return QLatin1String("Invites");
        default:
            return {};
        }
    };
};

/**
 * @class PushRuleModel
 *
 * This class defines the model for managing notification push rule keywords.
 */
class PushRuleModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The default state for any newly created keyword rule.
     */
    Q_PROPERTY(PushNotificationAction::Action defaultState READ defaultState WRITE setDefaultState NOTIFY defaultStateChanged)

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
     * @sa globalNotificationsEnabled, PushNotificationAction::Action
     */
    Q_PROPERTY(bool globalNotificationsSet READ globalNotificationsSet NOTIFY globalNotificationsSetChanged)

public:
    struct Rule {
        QString id;
        PushNotificationKind::Kind kind;
        PushNotificationAction::Action action;
        PushNotificationSection::Section section;
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
        ActionRole, /**< The PushNotificationAction for the rule. */
        HighlightableRole, /**< Whether the rule can have a highlight action. */
        DeletableRole, /**< Whether the rule can be deleted the rule. */
        SectionRole, /**< The section to sort into in the settings page. */
        RoomIdRole, /**< The room the rule applies to (blank if global). */
    };
    Q_ENUM(EventRoles)

    PushRuleModel(QObject *parent = nullptr);

    [[nodiscard]] PushNotificationAction::Action defaultState() const;
    void setDefaultState(PushNotificationAction::Action defaultState);

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

    Q_INVOKABLE void setPushRuleAction(const QString &id, PushNotificationAction::Action action);

    /**
     * @brief Add a new keyword to the model.
     */
    Q_INVOKABLE void addKeyword(const QString &keyword, const QString &roomId = {});

    /**
     * @brief Remove a keyword from the model.
     */
    Q_INVOKABLE void removeKeyword(const QString &keyword);

Q_SIGNALS:
    void defaultStateChanged();
    void globalNotificationsEnabledChanged();
    void globalNotificationsSetChanged();

private Q_SLOTS:
    void controllerConnectionChanged();
    void updateNotificationRules(const QString &type);

private:
    PushNotificationAction::Action m_defaultKeywordAction;
    QList<Rule> m_rules;

    void setRules(QVector<Quotient::PushRule> rules, PushNotificationKind::Kind kind);

    int getRuleIndex(const QString &ruleId) const;
    PushNotificationSection::Section getSection(Quotient::PushRule rule);

    void setNotificationRuleEnabled(const QString &kind, const QString &ruleId, bool enabled);
    void setNotificationRuleActions(const QString &kind, const QString &ruleId, PushNotificationAction::Action action);
    PushNotificationAction::Action variantToAction(const QVector<QVariant> &actions, bool enabled);
    QVector<QVariant> actionToVariant(PushNotificationAction::Action action, const QString &sound = "default");
};
Q_DECLARE_METATYPE(PushRuleModel *)
