// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "actionsmodel.h"

#include "chatbarcache.h"
#include "neochatroom.h"
#include "roommanager.h"
#include <Quotient/events/roommemberevent.h>
#include <Quotient/events/roompowerlevelsevent.h>

#include <KLocalizedString>

using Action = ActionsModel::Action;
using namespace Quotient;

QStringList rainbowColors{"#ff2b00"_ls, "#ff5500"_ls, "#ff8000"_ls, "#ffaa00"_ls, "#ffd500"_ls, "#ffff00"_ls, "#d4ff00"_ls, "#aaff00"_ls, "#80ff00"_ls,
                          "#55ff00"_ls, "#2bff00"_ls, "#00ff00"_ls, "#00ff2b"_ls, "#00ff55"_ls, "#00ff80"_ls, "#00ffaa"_ls, "#00ffd5"_ls, "#00ffff"_ls,
                          "#00d4ff"_ls, "#00aaff"_ls, "#007fff"_ls, "#0055ff"_ls, "#002bff"_ls, "#0000ff"_ls, "#2a00ff"_ls, "#5500ff"_ls, "#7f00ff"_ls,
                          "#aa00ff"_ls, "#d400ff"_ls, "#ff00ff"_ls, "#ff00d4"_ls, "#ff00aa"_ls, "#ff0080"_ls, "#ff0055"_ls, "#ff002b"_ls, "#ff0000"_ls};

auto leaveRoomLambda = [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
    if (text.isEmpty()) {
        Q_EMIT room->showMessage(NeoChatRoom::Info, i18n("Leaving this room."));
        room->connection()->leaveRoom(room);
    } else {
        QRegularExpression roomRegex(QStringLiteral(R"(^[#!][^:]+:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?)"));
        auto regexMatch = roomRegex.match(text);
        if (!regexMatch.hasMatch()) {
            Q_EMIT room->showMessage(NeoChatRoom::Error,
                                     i18nc("'<text>' does not look like a room id or alias.", "'%1' does not look like a room id or alias.", text));
            return QString();
        }
        auto leaving = room->connection()->room(text);
        if (!leaving) {
            leaving = room->connection()->roomByAlias(text);
        }
        if (leaving) {
            Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("Leaving room <roomname>.", "Leaving room %1.", text));
            room->connection()->leaveRoom(leaving);
        } else {
            Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("Room <roomname> not found", "Room %1 not found.", text));
        }
    }
    return QString();
};

auto roomNickLambda = [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
    if (text.isEmpty()) {
        Q_EMIT room->showMessage(NeoChatRoom::Error, i18n("No new nickname provided, no changes will happen."));
    } else {
        room->connection()->user()->rename(text, room);
    }
    return QString();
};

QList<ActionsModel::Action> actions{
    Action{
        QStringLiteral("shrug"),
        [](const QString &message, NeoChatRoom *, ChatBarCache *) {
            return QStringLiteral("¯\\\\_(ツ)_/¯ %1").arg(message);
        },
        true,
        std::nullopt,
        kli18n("<message>"),
        kli18n("Prepends ¯\\_(ツ)_/¯ to a plain-text message"),
    },
    Action{
        QStringLiteral("lenny"),
        [](const QString &message, NeoChatRoom *, ChatBarCache *) {
            return QStringLiteral("( ͡° ͜ʖ ͡°) %1").arg(message);
        },
        true,
        std::nullopt,
        kli18n("<message>"),
        kli18n("Prepends ( ͡° ͜ʖ ͡°) to a plain-text message"),
    },
    Action{
        QStringLiteral("tableflip"),
        [](const QString &message, NeoChatRoom *, ChatBarCache *) {
            return QStringLiteral("(╯°□°）╯︵ ┻━┻ %1").arg(message);
        },
        true,
        std::nullopt,
        kli18n("<message>"),
        kli18n("Prepends (╯°□°）╯︵ ┻━┻ to a plain-text message"),
    },
    Action{
        QStringLiteral("unflip"),
        [](const QString &message, NeoChatRoom *, ChatBarCache *) {
            return QStringLiteral("┬──┬ ノ( ゜-゜ノ) %1").arg(message);
        },
        true,
        std::nullopt,
        kli18n("<message>"),
        kli18n("Prepends ┬──┬ ノ( ゜-゜ノ) to a plain-text message"),
    },
    Action{
        QStringLiteral("rainbow"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *chatBarCache) {
            QString rainbowText;
            for (int i = 0; i < text.length(); i++) {
                rainbowText += QStringLiteral("<font color='%2'>%3</font>").arg(rainbowColors[i % rainbowColors.length()], text.at(i));
            }
            // Ideally, we would just return rainbowText and let that do the rest, but the colors don't survive markdownToHTML.
            room->postMessage(QStringLiteral("/rainbow %1").arg(text),
                              rainbowText,
                              RoomMessageEvent::MsgType::Text,
                              chatBarCache->replyId(),
                              chatBarCache->editId());
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<message>"),
        kli18n("Sends the given message colored as a rainbow"),
    },
    Action{
        QStringLiteral("rainbowme"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *chatBarCache) {
            QString rainbowText;
            for (int i = 0; i < text.length(); i++) {
                rainbowText += QStringLiteral("<font color='%2'>%3</font>").arg(rainbowColors[i % rainbowColors.length()], text.at(i));
            }
            // Ideally, we would just return rainbowText and let that do the rest, but the colors don't survive markdownToHTML.
            room->postMessage(QStringLiteral("/rainbow %1").arg(text),
                              rainbowText,
                              RoomMessageEvent::MsgType::Emote,
                              chatBarCache->replyId(),
                              chatBarCache->editId());
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<message>"),
        kli18n("Sends the given emote colored as a rainbow"),
    },
    Action{
        QStringLiteral("plain"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            room->postMessage(text, text.toHtmlEscaped(), RoomMessageEvent::MsgType::Text, {}, {});
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<message>"),
        kli18n("Sends the given message as plain text"),
    },
    Action{
        QStringLiteral("spoiler"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *chatBarCache) {
            // Ideally, we would just return rainbowText and let that do the rest, but the colors don't survive markdownToHTML.
            room->postMessage(QStringLiteral("/spoiler %1").arg(text),
                              QStringLiteral("<span data-mx-spoiler>%1</span>").arg(text),
                              RoomMessageEvent::MsgType::Text,
                              chatBarCache->replyId(),
                              chatBarCache->editId());
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<message>"),
        kli18n("Sends the given message as a spoiler"),
    },
    Action{
        QStringLiteral("me"),
        [](const QString &text, NeoChatRoom *, ChatBarCache *) {
            return text;
        },
        true,
        RoomMessageEvent::MsgType::Emote,
        kli18n("<message>"),
        kli18n("Sends the given emote"),
    },
    Action{
        QStringLiteral("notice"),
        [](const QString &text, NeoChatRoom *, ChatBarCache *) {
            return text;
        },
        true,
        RoomMessageEvent::MsgType::Notice,
        kli18n("<message>"),
        kli18n("Sends the given message as a notice"),
    },
    Action{
        QStringLiteral("invite"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            static const QRegularExpression mxidRegex(
                QStringLiteral(R"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"));
            auto regexMatch = mxidRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            const RoomMemberEvent *roomMemberEvent = room->currentState().get<RoomMemberEvent>(text);
            if (roomMemberEvent && roomMemberEvent->membership() == Membership::Invite) {
                Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("<user> is already invited to this room.", "%1 is already invited to this room.", text));
                return QString();
            }
            if (roomMemberEvent && roomMemberEvent->membership() == Membership::Ban) {
                Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("<user> is banned from this room.", "%1 is banned from this room.", text));
                return QString();
            }
            if (room->localUser()->id() == text) {
                Q_EMIT room->showMessage(NeoChatRoom::Positive, i18n("You are already in this room."));
                return QString();
            }
            if (room->users().contains(room->user(text))) {
                Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("<user> is already in this room.", "%1 is already in this room.", text));
                return QString();
            }
            room->inviteToRoom(text);
            Q_EMIT room->showMessage(NeoChatRoom::Positive, i18nc("<username> was invited into this room", "%1 was invited into this room", text));
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<user id>"),
        kli18n("Invites the user to this room"),
    },
    Action{
        QStringLiteral("join"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            QRegularExpression roomRegex(QStringLiteral(R"(^[#!][^:]+:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?)"));
            auto regexMatch = roomRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error,
                                         i18nc("'<text>' does not look like a room id or alias.", "'%1' does not look like a room id or alias.", text));
                return QString();
            }
            auto targetRoom = text.startsWith(QLatin1Char('!')) ? room->connection()->room(text) : room->connection()->roomByAlias(text);
            if (targetRoom) {
                RoomManager::instance().enterRoom(dynamic_cast<NeoChatRoom *>(targetRoom));
                return QString();
            }
            Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("Joining room <roomname>.", "Joining room %1.", text));
            RoomManager::instance().resolveResource(text, "join"_ls);
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<room alias or id>"),
        kli18n("Joins the given room"),
    },
    Action{
        QStringLiteral("knock"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            auto parts = text.split(QLatin1String(" "));
            QString roomName = parts[0];
            QRegularExpression roomRegex(QStringLiteral(R"(^[#!][^:]+:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?)"));
            auto regexMatch = roomRegex.match(roomName);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error,
                                         i18nc("'<text>' does not look like a room id or alias.", "'%1' does not look like a room id or alias.", text));
                return QString();
            }
            auto targetRoom = text.startsWith(QLatin1Char('!')) ? room->connection()->room(text) : room->connection()->roomByAlias(text);
            if (targetRoom) {
                RoomManager::instance().enterRoom(dynamic_cast<NeoChatRoom *>(targetRoom));
                return QString();
            }
            Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("Knocking room <roomname>.", "Knocking room %1.", text));
            auto connection = room->connection();
            const auto knownServer = roomName.mid(roomName.indexOf(":"_ls) + 1);
            if (parts.length() >= 2) {
                RoomManager::instance().knockRoom(connection, roomName, parts[1], QStringList{knownServer});
            } else {
                RoomManager::instance().knockRoom(connection, roomName, QString(), QStringList{knownServer});
            }
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<room alias or id> [<reason>]"),
        kli18n("Requests to join the given room"),
    },
    Action{
        QStringLiteral("j"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            QRegularExpression roomRegex(QStringLiteral(R"(^[#!][^:]+:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?)"));
            auto regexMatch = roomRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error,
                                         i18nc("'<text>' does not look like a room id or alias.", "'%1' does not look like a room id or alias.", text));
                return QString();
            }
            if (room->connection()->room(text) || room->connection()->roomByAlias(text)) {
                Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("You are already in room <roomname>.", "You are already in room %1.", text));
                return QString();
            }
            Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("Joining room <roomname>.", "Joining room %1.", text));
            RoomManager::instance().resolveResource(text, "join"_ls);
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<room alias or id>"),
        kli18n("Joins the given room"),
    },
    Action{
        QStringLiteral("part"),
        leaveRoomLambda,
        false,
        std::nullopt,
        kli18n("[<room alias or id>]"),
        kli18n("Leaves the given room or this room, if there is none given"),
    },
    Action{
        QStringLiteral("leave"),
        leaveRoomLambda,
        false,
        std::nullopt,
        kli18n("[<room alias or id>]"),
        kli18n("Leaves the given room or this room, if there is none given"),
    },
    Action{
        QStringLiteral("nick"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            if (text.isEmpty()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18n("No new nickname provided, no changes will happen."));
            } else {
                room->connection()->user()->rename(text);
            }
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<display name>"),
        kli18n("Changes your global display name"),
    },
    Action{
        QStringLiteral("roomnick"),
        roomNickLambda,
        false,
        std::nullopt,
        kli18n("<display name>"),
        kli18n("Changes your display name in this room"),
    },
    Action{
        QStringLiteral("myroomnick"),
        roomNickLambda,
        false,
        std::nullopt,
        kli18n("<display name>"),
        kli18n("Changes your display name in this room"),
    },
    Action{
        QStringLiteral("ignore"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            static const QRegularExpression mxidRegex(
                QStringLiteral(R"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"));
            auto regexMatch = mxidRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            if (room->connection()->ignoredUsers().contains(text)) {
                Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("<username> is already ignored.", "%1 is already ignored.", text));
                return QString();
            }
            room->connection()->addToIgnoredUsers(room->connection()->user(text));
            Q_EMIT room->showMessage(NeoChatRoom::Positive, i18nc("<username> is now ignored", "%1 is now ignored.", text));
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<user id>"),
        kli18n("Ignores the given user"),
    },
    Action{
        QStringLiteral("unignore"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            static const QRegularExpression mxidRegex(
                QStringLiteral(R"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"));
            auto regexMatch = mxidRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            if (!room->connection()->ignoredUsers().contains(text)) {
                Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("<username> is not ignored.", "%1 is not ignored.", text));
                return QString();
            }
            room->connection()->removeFromIgnoredUsers(room->connection()->user(text));
            Q_EMIT room->showMessage(NeoChatRoom::Positive, i18nc("<username> is no longer ignored.", "%1 is no longer ignored.", text));
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<user id>"),
        kli18n("Unignores the given user"),
    },
    Action{
        QStringLiteral("react"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *chatBarCache) {
            if (chatBarCache->replyId().isEmpty()) {
                for (auto it = room->messageEvents().crbegin(); it != room->messageEvents().crend(); it++) {
                    const auto &evt = **it;
                    if (const auto event = eventCast<const RoomMessageEvent>(&evt)) {
                        room->toggleReaction(event->id(), text);
                        return QString();
                    }
                }
            }
            room->toggleReaction(chatBarCache->replyId(), text);
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<reaction text>"),
        kli18n("React to the message with the given text"),
    },
    Action{
        QStringLiteral("ban"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            auto parts = text.split(QLatin1String(" "));
            static const QRegularExpression mxidRegex(
                QStringLiteral(R"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"));
            auto regexMatch = mxidRegex.match(parts[0]);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            auto state = room->currentState().get<RoomMemberEvent>(parts[0]);
            if (state && state->membership() == Membership::Ban) {
                Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("<user> is already banned from this room.", "%1 is already banned from this room.", text));
                return QString();
            }
            auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
            if (!plEvent) {
                return QString();
            }
            if (plEvent->ban() > plEvent->powerLevelForUser(room->localUser()->id())) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18n("You are not allowed to ban users from this room."));
                return QString();
            }
            if (plEvent->powerLevelForUser(room->localUser()->id()) <= plEvent->powerLevelForUser(parts[0])) {
                Q_EMIT room->showMessage(
                    NeoChatRoom::Error,
                    i18nc("You are not allowed to ban <username> from this room.", "You are not allowed to ban %1 from this room.", parts[0]));
                return QString();
            }
            room->ban(parts[0], parts.size() > 1 ? parts.mid(1).join(QLatin1Char(' ')) : QString());
            Q_EMIT room->showMessage(NeoChatRoom::Positive, i18nc("<username> was banned from this room.", "%1 was banned from this room.", parts[0]));
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<user id> [<reason>]"),
        kli18n("Bans the given user"),
    },
    Action{
        QStringLiteral("unban"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            static const QRegularExpression mxidRegex(
                QStringLiteral(R"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"));
            auto regexMatch = mxidRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
            if (!plEvent) {
                return QString();
            }
            if (plEvent->ban() > plEvent->powerLevelForUser(room->localUser()->id())) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18n("You are not allowed to unban users from this room."));
                return QString();
            }
            auto state = room->currentState().get<RoomMemberEvent>(text);
            if (state && state->membership() != Membership::Ban) {
                Q_EMIT room->showMessage(NeoChatRoom::Info, i18nc("<user> is not banned from this room.", "%1 is not banned from this room.", text));
                return QString();
            }
            room->unban(text);
            Q_EMIT room->showMessage(NeoChatRoom::Positive, i18nc("<username> was unbanned from this room.", "%1 was unbanned from this room.", text));

            return QString();
        },
        false,
        std::nullopt,
        kli18n("<user id>"),
        kli18n("Removes the ban of the given user"),
    },
    Action{
        QStringLiteral("kick"),
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            auto parts = text.split(QLatin1String(" "));
            static const QRegularExpression mxidRegex(
                QStringLiteral(R"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"));
            auto regexMatch = mxidRegex.match(parts[0]);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error,
                                         i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", parts[0]));
                return QString();
            }
            if (parts[0] == room->localUser()->id()) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18n("You cannot kick yourself from the room."));
                return QString();
            }
            if (!room->isMember(parts[0])) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18nc("<username> is not in this room", "%1 is not in this room.", parts[0]));
                return QString();
            }
            auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
            if (!plEvent) {
                return QString();
            }
            auto kick = plEvent->kick();
            if (plEvent->powerLevelForUser(room->localUser()->id()) < kick) {
                Q_EMIT room->showMessage(NeoChatRoom::Error, i18n("You are not allowed to kick users from this room."));
                return QString();
            }
            if (plEvent->powerLevelForUser(room->localUser()->id()) <= plEvent->powerLevelForUser(parts[0])) {
                Q_EMIT room->showMessage(
                    NeoChatRoom::Error,
                    i18nc("You are not allowed to kick <username> from this room", "You are not allowed to kick %1 from this room.", parts[0]));
                return QString();
            }
            room->kickMember(parts[0], parts.size() > 1 ? parts.mid(1).join(QLatin1Char(' ')) : QString());
            Q_EMIT room->showMessage(NeoChatRoom::Positive, i18nc("<username> was kicked from this room.", "%1 was kicked from this room.", parts[0]));
            return QString();
        },
        false,
        std::nullopt,
        kli18n("<user id> [<reason>]"),
        kli18n("Removes the user from the room"),
    },
};

int ActionsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return actions.size();
}

QVariant ActionsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= actions.size()) {
        return {};
    }
    if (role == Prefix) {
        return actions[index.row()].prefix;
    }
    if (role == Description) {
        return actions[index.row()].description.toString();
    }
    if (role == CompletionType) {
        return QStringLiteral("action");
    }
    if (role == Parameters) {
        return actions[index.row()].parameters.toString();
    }
    return {};
}

QHash<int, QByteArray> ActionsModel::roleNames() const
{
    return {
        {Prefix, "prefix"},
        {Description, "description"},
        {CompletionType, "completionType"},
    };
}

QList<Action> &ActionsModel::allActions() const
{
    return actions;
}
