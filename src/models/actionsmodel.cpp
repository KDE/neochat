// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "actionsmodel.h"

#include "chatbarcache.h"
#include "enums/messagetype.h"
#include "neochatconfig.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "roommanager.h"
#include <Quotient/events/eventcontent.h>
#include <Quotient/events/roommemberevent.h>
#include <Quotient/events/roompowerlevelsevent.h>
#include <Quotient/user.h>

#include <KLocalizedString>

using Action = ActionsModel::Action;
using namespace Quotient;
using namespace Qt::StringLiterals;

QStringList rainbowColors{"#ff2b00"_L1, "#ff5500"_L1, "#ff8000"_L1, "#ffaa00"_L1, "#ffd500"_L1, "#ffff00"_L1, "#d4ff00"_L1, "#aaff00"_L1, "#80ff00"_L1,
                          "#55ff00"_L1, "#2bff00"_L1, "#00ff00"_L1, "#00ff2b"_L1, "#00ff55"_L1, "#00ff80"_L1, "#00ffaa"_L1, "#00ffd5"_L1, "#00ffff"_L1,
                          "#00d4ff"_L1, "#00aaff"_L1, "#007fff"_L1, "#0055ff"_L1, "#002bff"_L1, "#0000ff"_L1, "#2a00ff"_L1, "#5500ff"_L1, "#7f00ff"_L1,
                          "#aa00ff"_L1, "#d400ff"_L1, "#ff00ff"_L1, "#ff00d4"_L1, "#ff00aa"_L1, "#ff0080"_L1, "#ff0055"_L1, "#ff002b"_L1, "#ff0000"_L1};

auto leaveRoomLambda = [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
    if (text.isEmpty()) {
        Q_EMIT room->showMessage(MessageType::Information, i18n("Leaving this room."));
        room->connection()->leaveRoom(room);
    } else {
        QRegularExpression roomRegex(uR"(^[#!][^:]+:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?)"_s);
        auto regexMatch = roomRegex.match(text);
        if (!regexMatch.hasMatch()) {
            Q_EMIT room->showMessage(MessageType::Error,
                                     i18nc("'<text>' does not look like a room id or alias.", "'%1' does not look like a room id or alias.", text));
            return QString();
        }
        auto leaving = room->connection()->room(text);
        if (!leaving) {
            leaving = room->connection()->roomByAlias(text);
        }
        if (leaving) {
            Q_EMIT room->showMessage(MessageType::Information, i18nc("Leaving room <roomname>.", "Leaving room %1.", text));
            room->connection()->leaveRoom(leaving);
        } else {
            Q_EMIT room->showMessage(MessageType::Information, i18nc("Room <roomname> not found", "Room %1 not found.", text));
        }
    }
    return QString();
};

auto roomNickLambda = [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
    if (text.isEmpty()) {
        Q_EMIT room->showMessage(MessageType::Error, i18n("No new nickname provided, no changes will happen."));
    } else {
        room->connection()->user()->rename(text, room);
    }
    return QString();
};

QList<ActionsModel::Action> actions{
    Action{
        u"shrug"_s,
        [](const QString &message, NeoChatRoom *, ChatBarCache *) {
            return u"¯\\\\_(ツ)_/¯ %1"_s.arg(message);
        },
        Quotient::RoomMessageEvent::MsgType::Text,
        kli18n("<message>"),
        kli18n("Prepends ¯\\_(ツ)_/¯ to a plain-text message"),
    },
    Action{
        u"lenny"_s,
        [](const QString &message, NeoChatRoom *, ChatBarCache *) {
            return u"( ͡° ͜ʖ ͡°) %1"_s.arg(message);
        },
        Quotient::RoomMessageEvent::MsgType::Text,
        kli18n("<message>"),
        kli18n("Prepends ( ͡° ͜ʖ ͡°) to a plain-text message"),
    },
    Action{
        u"tableflip"_s,
        [](const QString &message, NeoChatRoom *, ChatBarCache *) {
            return u"(╯°□°）╯︵ ┻━┻ %1"_s.arg(message);
        },
        Quotient::RoomMessageEvent::MsgType::Text,
        kli18n("<message>"),
        kli18n("Prepends (╯°□°）╯︵ ┻━┻ to a plain-text message"),
    },
    Action{
        u"unflip"_s,
        [](const QString &message, NeoChatRoom *, ChatBarCache *) {
            return u"┬──┬ ノ( ゜-゜ノ) %1"_s.arg(message);
        },
        Quotient::RoomMessageEvent::MsgType::Text,
        kli18n("<message>"),
        kli18n("Prepends ┬──┬ ノ( ゜-゜ノ) to a plain-text message"),
    },
    Action{
        u"rainbow"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *chatBarCache) {
            QString rainbowText;
            for (int i = 0; i < text.length(); i++) {
                rainbowText += u"<font color='%2'>%3</font>"_s.arg(rainbowColors[i % rainbowColors.length()], text.at(i));
            }
            // Ideally, we would just return rainbowText and let that do the rest, but the colors don't survive markdownToHTML.
            auto content = std::make_unique<Quotient::EventContent::TextContent>(rainbowText, u"text/html"_s);
            EventRelation relatesTo =
                chatBarCache->isReplying() ? EventRelation::replyTo(chatBarCache->replyId()) : EventRelation::replace(chatBarCache->editId());
            room->post<Quotient::RoomMessageEvent>("/rainbow %1"_L1.arg(text), MessageEventType::Text, std::move(content), relatesTo);
            return QString();
        },
        std::nullopt,
        kli18n("<message>"),
        kli18n("Sends the given message colored as a rainbow"),
    },
    Action{
        u"rainbowme"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *chatBarCache) {
            QString rainbowText;
            for (int i = 0; i < text.length(); i++) {
                rainbowText += u"<font color='%2'>%3</font>"_s.arg(rainbowColors[i % rainbowColors.length()], text.at(i));
            }
            // Ideally, we would just return rainbowText and let that do the rest, but the colors don't survive markdownToHTML.
            auto content = std::make_unique<Quotient::EventContent::TextContent>(rainbowText, u"text/html"_s);
            EventRelation relatesTo =
                chatBarCache->isReplying() ? EventRelation::replyTo(chatBarCache->replyId()) : EventRelation::replace(chatBarCache->editId());
            room->post<Quotient::RoomMessageEvent>(u"/rainbow %1"_s.arg(text), MessageEventType::Emote, std::move(content), relatesTo);
            return QString();
        },
        std::nullopt,
        kli18n("<message>"),
        kli18n("Sends the given emote colored as a rainbow"),
    },
    Action{
        u"plain"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
#if Quotient_VERSION_MINOR >= 9
            room->postText(text.toHtmlEscaped());
#else
            room->postPlainText(text.toHtmlEscaped());
#endif
            return QString();
        },
        std::nullopt,
        kli18n("<message>"),
        kli18n("Sends the given message as plain text"),
    },
    Action{
        u"spoiler"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *chatBarCache) {
            // Ideally, we would just return rainbowText and let that do the rest, but the colors don't survive markdownToHTML.
            auto content = std::make_unique<Quotient::EventContent::TextContent>(u"<span data-mx-spoiler>%1</span>"_s.arg(text), u"text/html"_s);
            EventRelation relatesTo =
                chatBarCache->isReplying() ? EventRelation::replyTo(chatBarCache->replyId()) : EventRelation::replace(chatBarCache->editId());
            room->post<Quotient::RoomMessageEvent>(u"/spoiler %1"_s.arg(text), MessageEventType::Text, std::move(content), relatesTo);
            return QString();
        },
        std::nullopt,
        kli18n("<message>"),
        kli18n("Sends the given message as a spoiler"),
    },
    Action{
        u"me"_s,
        [](const QString &text, NeoChatRoom *, ChatBarCache *) {
            return text;
        },
        RoomMessageEvent::MsgType::Emote,
        kli18n("<message>"),
        kli18n("Sends the given emote"),
    },
    Action{
        u"notice"_s,
        [](const QString &text, NeoChatRoom *, ChatBarCache *) {
            return text;
        },
        RoomMessageEvent::MsgType::Notice,
        kli18n("<message>"),
        kli18n("Sends the given message as a notice"),
    },
    Action{
        u"invite"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            static const QRegularExpression mxidRegex(uR"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"_s);
            auto regexMatch = mxidRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(MessageType::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            const RoomMemberEvent *roomMemberEvent = room->currentState().get<RoomMemberEvent>(text);
            if (roomMemberEvent && roomMemberEvent->membership() == Membership::Invite) {
                Q_EMIT room->showMessage(MessageType::Information,
                                         i18nc("<user> is already invited to this room.", "%1 is already invited to this room.", text));
                return QString();
            }
            if (roomMemberEvent && roomMemberEvent->membership() == Membership::Ban) {
                Q_EMIT room->showMessage(MessageType::Information, i18nc("<user> is banned from this room.", "%1 is banned from this room.", text));
                return QString();
            }
            if (room->localMember().id() == text) {
                Q_EMIT room->showMessage(MessageType::Positive, i18n("You are already in this room."));
                return QString();
            }
            if (room->joinedMemberIds().contains(text)) {
                Q_EMIT room->showMessage(MessageType::Information, i18nc("<user> is already in this room.", "%1 is already in this room.", text));
                return QString();
            }
            room->inviteToRoom(text);
            Q_EMIT room->showMessage(MessageType::Positive, i18nc("<username> was invited into this room.", "%1 was invited into this room.", text));
            return QString();
        },
        std::nullopt,
        kli18n("<user id>"),
        kli18n("Invites the user to this room"),
    },
    Action{
        u"join"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            QRegularExpression roomRegex(uR"(^[#!][^:]+:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?)"_s);
            auto regexMatch = roomRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(MessageType::Error,
                                         i18nc("'<text>' does not look like a room id or alias.", "'%1' does not look like a room id or alias.", text));
                return QString();
            }
            auto targetRoom = text.startsWith(QLatin1Char('!')) ? room->connection()->room(text) : room->connection()->roomByAlias(text);
            if (targetRoom) {
                RoomManager::instance().resolveResource(targetRoom->id());
                return QString();
            }
            Q_EMIT room->showMessage(MessageType::Information, i18nc("Joining room <roomname>.", "Joining room %1.", text));
            RoomManager::instance().resolveResource(text, "join"_L1);
            return QString();
        },
        std::nullopt,
        kli18n("<room alias or id>"),
        kli18n("Joins the given room"),
    },
    Action{
        u"knock"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            auto parts = text.split(u" "_s);
            QString roomName = parts[0];
            QRegularExpression roomRegex(uR"(^[#!][^:]+:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?)"_s);
            auto regexMatch = roomRegex.match(roomName);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(MessageType::Error,
                                         i18nc("'<text>' does not look like a room id or alias.", "'%1' does not look like a room id or alias.", text));
                return QString();
            }
            auto targetRoom = text.startsWith(QLatin1Char('!')) ? room->connection()->room(text) : room->connection()->roomByAlias(text);
            if (targetRoom) {
                RoomManager::instance().resolveResource(targetRoom->id());
                return QString();
            }
            Q_EMIT room->showMessage(MessageType::Information, i18nc("Knocking room <roomname>.", "Knocking room %1.", text));
            auto connection = dynamic_cast<NeoChatConnection *>(room->connection());
            const auto knownServer = roomName.mid(roomName.indexOf(":"_L1) + 1);
            if (parts.length() >= 2) {
                RoomManager::instance().knockRoom(connection, roomName, parts[1], QStringList{knownServer});
            } else {
                RoomManager::instance().knockRoom(connection, roomName, QString(), QStringList{knownServer});
            }
            return QString();
        },
        std::nullopt,
        kli18n("<room alias or id> [<reason>]"),
        kli18n("Requests to join the given room"),
    },
    Action{
        u"j"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            QRegularExpression roomRegex(uR"(^[#!][^:]+:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?)"_s);
            auto regexMatch = roomRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(MessageType::Error,
                                         i18nc("'<text>' does not look like a room id or alias.", "'%1' does not look like a room id or alias.", text));
                return QString();
            }
            if (room->connection()->room(text) || room->connection()->roomByAlias(text)) {
                Q_EMIT room->showMessage(MessageType::Information, i18nc("You are already in room <roomname>.", "You are already in room %1.", text));
                return QString();
            }
            Q_EMIT room->showMessage(MessageType::Information, i18nc("Joining room <roomname>.", "Joining room %1.", text));
            RoomManager::instance().resolveResource(text, "join"_L1);
            return QString();
        },
        std::nullopt,
        kli18n("<room alias or id>"),
        kli18n("Joins the given room"),
    },
    Action{
        u"part"_s,
        leaveRoomLambda,
        std::nullopt,
        kli18n("[<room alias or id>]"),
        kli18n("Leaves the given room or this room, if there is none given"),
    },
    Action{
        u"leave"_s,
        leaveRoomLambda,
        std::nullopt,
        kli18n("[<room alias or id>]"),
        kli18n("Leaves the given room or this room, if there is none given"),
    },
    Action{
        u"nick"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            if (text.isEmpty()) {
                Q_EMIT room->showMessage(MessageType::Error, i18n("No new nickname provided, no changes will happen."));
            } else {
                room->connection()->user()->rename(text);
            }
            return QString();
        },
        std::nullopt,
        kli18n("<display name>"),
        kli18n("Changes your global display name"),
    },
    Action{
        u"roomnick"_s,
        roomNickLambda,
        std::nullopt,
        kli18n("<display name>"),
        kli18n("Changes your display name in this room"),
    },
    Action{
        u"myroomnick"_s,
        roomNickLambda,
        std::nullopt,
        kli18n("<display name>"),
        kli18n("Changes your display name in this room"),
    },
    Action{
        u"ignore"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            static const QRegularExpression mxidRegex(uR"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"_s);
            auto regexMatch = mxidRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(MessageType::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            if (room->connection()->ignoredUsers().contains(text)) {
                Q_EMIT room->showMessage(MessageType::Information, i18nc("<username> is already ignored.", "%1 is already ignored.", text));
                return QString();
            }
            room->connection()->addToIgnoredUsers(text);
            Q_EMIT room->showMessage(MessageType::Positive, i18nc("<username> is now ignored", "%1 is now ignored.", text));

            return QString();
        },
        std::nullopt,
        kli18n("<user id>"),
        kli18n("Ignores the given user"),
    },
    Action{
        u"unignore"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            static const QRegularExpression mxidRegex(uR"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"_s);
            auto regexMatch = mxidRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(MessageType::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            if (!room->connection()->ignoredUsers().contains(text)) {
                Q_EMIT room->showMessage(MessageType::Information, i18nc("<username> is not ignored.", "%1 is not ignored.", text));
                return QString();
            }
            room->connection()->removeFromIgnoredUsers(text);
            Q_EMIT room->showMessage(MessageType::Positive, i18nc("<username> is no longer ignored.", "%1 is no longer ignored.", text));
            return QString();
        },
        std::nullopt,
        kli18n("<user id>"),
        kli18n("Unignores the given user"),
    },
    Action{
        u"react"_s,
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
        std::nullopt,
        kli18n("<reaction text>"),
        kli18n("React to the message with the given text"),
    },
    Action{
        u"ban"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            auto parts = text.split(u" "_s);
            static const QRegularExpression mxidRegex(uR"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"_s);
            auto regexMatch = mxidRegex.match(parts[0]);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(MessageType::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            auto state = room->currentState().get<RoomMemberEvent>(parts[0]);
            if (state && state->membership() == Membership::Ban) {
                Q_EMIT room->showMessage(MessageType::Information,
                                         i18nc("<user> is already banned from this room.", "%1 is already banned from this room.", text));
                return QString();
            }
            auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
            if (!plEvent) {
                return QString();
            }
            if (plEvent->ban() > plEvent->powerLevelForUser(room->localMember().id())) {
                Q_EMIT room->showMessage(MessageType::Error, i18n("You are not allowed to ban users from this room."));
                return QString();
            }
            if (plEvent->powerLevelForUser(room->localMember().id()) <= plEvent->powerLevelForUser(parts[0])) {
                Q_EMIT room->showMessage(
                    MessageType::Error,
                    i18nc("You are not allowed to ban <username> from this room.", "You are not allowed to ban %1 from this room.", parts[0]));
                return QString();
            }
            room->ban(parts[0], parts.size() > 1 ? parts.mid(1).join(QLatin1Char(' ')) : QString());
            Q_EMIT room->showMessage(MessageType::Positive, i18nc("<username> was banned from this room.", "%1 was banned from this room.", parts[0]));
            return QString();
        },
        std::nullopt,
        kli18n("<user id> [<reason>]"),
        kli18n("Bans the given user"),
    },
    Action{
        u"unban"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            static const QRegularExpression mxidRegex(uR"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"_s);
            auto regexMatch = mxidRegex.match(text);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(MessageType::Error, i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", text));
                return QString();
            }
            auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
            if (!plEvent) {
                return QString();
            }
            if (plEvent->ban() > plEvent->powerLevelForUser(room->localMember().id())) {
                Q_EMIT room->showMessage(MessageType::Error, i18n("You are not allowed to unban users from this room."));
                return QString();
            }
            auto state = room->currentState().get<RoomMemberEvent>(text);
            if (state && state->membership() != Membership::Ban) {
                Q_EMIT room->showMessage(MessageType::Information, i18nc("<user> is not banned from this room.", "%1 is not banned from this room.", text));
                return QString();
            }
            room->unban(text);
            Q_EMIT room->showMessage(MessageType::Positive, i18nc("<username> was unbanned from this room.", "%1 was unbanned from this room.", text));

            return QString();
        },
        std::nullopt,
        kli18n("<user id>"),
        kli18n("Removes the ban of the given user"),
    },
    Action{
        u"kick"_s,
        [](const QString &text, NeoChatRoom *room, ChatBarCache *) {
            auto parts = text.split(u" "_s);
            static const QRegularExpression mxidRegex(uR"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"_s);
            auto regexMatch = mxidRegex.match(parts[0]);
            if (!regexMatch.hasMatch()) {
                Q_EMIT room->showMessage(MessageType::Error,
                                         i18nc("'<text>' does not look like a matrix id.", "'%1' does not look like a matrix id.", parts[0]));
                return QString();
            }
            if (parts[0] == room->localMember().id()) {
                Q_EMIT room->showMessage(MessageType::Error, i18n("You cannot kick yourself from the room."));
                return QString();
            }
            if (!room->isMember(parts[0])) {
                Q_EMIT room->showMessage(MessageType::Error, i18nc("<username> is not in this room", "%1 is not in this room.", parts[0]));
                return QString();
            }
            auto plEvent = room->currentState().get<RoomPowerLevelsEvent>();
            if (!plEvent) {
                return QString();
            }
            auto kick = plEvent->kick();
            if (plEvent->powerLevelForUser(room->localMember().id()) < kick) {
                Q_EMIT room->showMessage(MessageType::Error, i18n("You are not allowed to kick users from this room."));
                return QString();
            }
            if (plEvent->powerLevelForUser(room->localMember().id()) <= plEvent->powerLevelForUser(parts[0])) {
                Q_EMIT room->showMessage(
                    MessageType::Error,
                    i18nc("You are not allowed to kick <username> from this room", "You are not allowed to kick %1 from this room.", parts[0]));
                return QString();
            }
            room->kickMember(parts[0], parts.size() > 1 ? parts.mid(1).join(QLatin1Char(' ')) : QString());
            Q_EMIT room->showMessage(MessageType::Positive, i18nc("<username> was kicked from this room.", "%1 was kicked from this room.", parts[0]));
            return QString();
        },
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
        return u"action"_s;
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

QList<Action> &ActionsModel::allActions()
{
    return actions;
}

bool ActionsModel::handleQuickEditAction(NeoChatRoom *room, const QString &messageText)
{
    if (room == nullptr) {
        return false;
    }

    if (NeoChatConfig::allowQuickEdit()) {
        QRegularExpression sed(u"^s/([^/]*)/([^/]*)(/g)?$"_s);
        auto match = sed.match(messageText);
        if (match.hasMatch()) {
            const QString regex = match.captured(1);
            const QString replacement = match.captured(2).toHtmlEscaped();
            const QString flags = match.captured(3);

            for (auto it = room->messageEvents().crbegin(); it != room->messageEvents().crend(); it++) {
                if (const auto event = eventCast<const RoomMessageEvent>(&**it)) {
                    if (event->senderId() == room->localMember().id() && event->has<EventContent::TextContent>()) {
                        QString originalString;
                        if (event->content()) {
                            originalString = static_cast<const Quotient::EventContent::TextContent *>(event->content().get())->body;
                        } else {
                            originalString = event->plainBody();
                        }
                        QString replaceId = event->id();
                        const auto eventRelation = event->relatesTo();
                        if (eventRelation && eventRelation->type == "m.replace"_L1) {
                            replaceId = eventRelation->eventId;
                        }

                        std::unique_ptr<EventContent::TextContent> content = nullptr;
                        if (flags == "/g"_L1) {
                            content = std::make_unique<Quotient::EventContent::TextContent>(originalString.replace(regex, replacement), u"text/html"_s);
                        } else {
                            content = std::make_unique<Quotient::EventContent::TextContent>(originalString.replace(regex, replacement), u"text/html"_s);
                        }
                        Quotient::EventRelation relatesTo = Quotient::EventRelation::replace(replaceId);
                        room->post<Quotient::RoomMessageEvent>(messageText, event->msgtype(), std::move(content), relatesTo);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

std::pair<std::optional<QString>, std::optional<Quotient::RoomMessageEvent::MsgType>> ActionsModel::handleAction(NeoChatRoom *room, ChatBarCache *chatBarCache)
{
    auto sendText = chatBarCache->sendText();
    const auto edited = handleQuickEditAction(room, sendText);
    if (edited) {
        return std::make_pair(std::nullopt, std::nullopt);
    }

    std::optional<Quotient::RoomMessageEvent::MsgType> messageType = Quotient::RoomMessageEvent::MsgType::Text;
    if (sendText.startsWith(QLatin1Char('/'))) {
        for (const auto &action : ActionsModel::instance().allActions()) {
            if (sendText.indexOf(action.prefix) == 1
                && (sendText.indexOf(" "_L1) == action.prefix.length() + 1 || sendText.length() == action.prefix.length() + 1)) {
                sendText = action.handle(sendText.mid(action.prefix.length() + 1).trimmed(), room, chatBarCache);
                if (action.messageType.has_value()) {
                    messageType = action.messageType;
                } else {
                    messageType = std::nullopt;
                }
            }
        }
    }

    return std::make_pair(messageType.has_value() ? std::make_optional(sendText) : std::nullopt, messageType);
}
