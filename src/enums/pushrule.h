// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

/**
 * @class PushRuleKind
 *
 * A class with the Kind enum for push notifications and helper functions.
 *
 * The kind relates to the kinds of push rule defined in the matrix spec, see
 * https://spec.matrix.org/v1.7/client-server-api/#push-rules for full details.
 */
class PushRuleKind : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

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
 * @class PushRuleAction
 *
 * A class with the Action enum for push notifications.
 *
 * The action relates to the actions of push rule defined in the matrix spec, see
 * https://spec.matrix.org/v1.7/client-server-api/#push-rules for full details.
 */
class PushRuleAction : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief Defines the global push notification actions.
     */
    enum Action {
        Unknown = 0, /**< The action has not yet been obtained from the server. */
        Off, /**< No push notifications are to be sent. */
        On, /**< Push notifications are on. */
        Noisy, /**< Push notifications are on, also trigger a notification sound. */
        Highlight, /**< Push notifications are on, also the event should be highlighted in chat. */
        NoisyHighlight, /**< Push notifications are on, also trigger a notification sound and highlight in chat. */
    };
    Q_ENUM(Action)
};

/**
 * @class PushNotificationState
 *
 * A class with the State enum for room push notification state.
 *
 * The state define whether the room adheres to the global push rule states for the
 * account or is overridden for a room.
 *
 * @note This is different to the PushRuleAction which defines the type of notification
 *       for an individual rule.
 *
 * @sa PushRuleAction
 */
class PushNotificationState : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief Describes the push notification state for the room.
     */
    enum State {
        Unknown, /**< The state has not yet been obtained from the server. */
        Default, /**< The room follows the globally configured rules for the local user. */
        Mute, /**< No notifications for messages in the room. */
        MentionKeyword, /**< Notifications only for local user mentions and keywords. */
        All, /**< Notifications for all messages. */
    };
    Q_ENUM(State)
};

/**
 * @class PushRuleSection
 *
 * A class with the Section enum for push notifications and helper functions.
 *
 * @note This is different from the PushRuleKind and instead is used for sorting
 *       in the settings page which is not necessarily by Kind.
 *
 * @sa PushRuleKind
 */
class PushRuleSection : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

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
        Unknown, /**< New default push rules that have not been added to the model yet. */
        /**
         * @brief Push rules that should never be shown.
         *
         * There are numerous rules that get set that shouldn't be shown in the general
         * list e.g. The array of rules used to override global settings in individual
         * rooms.
         *
         * This is specifically different to unknown which are just new default push
         * rule that haven't been added to the model yet.
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
