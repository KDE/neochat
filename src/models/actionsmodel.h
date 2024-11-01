// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <KLazyLocalizedString>
#include <QAbstractListModel>
#include <Quotient/events/roommessageevent.h>

class ChatBarCache;
class NeoChatRoom;

/**
 * @class ActionsModel
 *
 * This class defines a model for chat actions.
 *
 * @note A chat action is a message starting with /, resulting in something other
 *       than a normal message being sent (e.g. /me, /join).
 */
class ActionsModel : public QAbstractListModel
{
public:
    /**
     * @brief Definition of an action.
     */
    struct Action {
        QString prefix; /**< The prefix, without '/' and space after the word. */
        /**
         * @brief The function to execute when the action is triggered.
         */
        std::function<QString(const QString &, NeoChatRoom *, ChatBarCache *)> handle;
        /**
         * @brief Whether the action is a message type action.
         *
         * If this is true, a message action will be sent. If this is false, this
         * message does some action on the client and should not be sent as a message.
         */
        bool messageAction;
        /**
         * @brief The new message type of a message being sent.
         *
         * For a non-message action or a message action that outputs the same type
         * as its input, it's nullopt.
         */
        std::optional<Quotient::RoomMessageEvent::MsgType> messageType = std::nullopt;
        KLazyLocalizedString parameters; /**< The input parameters expected by the action. */
        KLazyLocalizedString description; /**< The description of the action. */
    };
    static ActionsModel &instance()
    {
        static ActionsModel _instance;
        return _instance;
    }

    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        Prefix = Qt::DisplayRole, /**< The prefix, without '/' and space after the word. */
        Description, /**< The description of the action. */
        CompletionType, /**< The completion type (always "action" for this model). */
        Parameters, /**< The input parameters expected by the action. */
    };
    Q_ENUM(Roles)

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Return a vector with all supported actions.
     */
    QList<Action> &allActions() const;

    /**
     * @brief Handle special sed style edit action.
     *
     * @return True if the message has a sed edit which was actioned. False otherwise.
     */
    static bool handleQuickEditAction(NeoChatRoom *room, const QString &messageText);

    /**
     * @brief Handle any action within the message contained in the given ChatBarCache.
     *
     * @return A modified or unmodified string that needs to be sent or an empty string if
     *         the handled action replaces sending a normal message.
     */
    static std::pair<std::optional<QString>, std::optional<Quotient::RoomMessageEvent::MsgType>> handleAction(NeoChatRoom *room, ChatBarCache *chatBarCache);

private:
    ActionsModel() = default;
};
