// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionshandler.h"
#include "controller.h"

#include <csapi/joining.h>
#include <events/roommessageevent.h>

#include <KLocalizedString>
#include <QDebug>
#include <QStringBuilder>

#include "controller.h"
#include "roommanager.h"
#include "customemojimodel.h"

ActionsHandler::ActionsHandler(QObject *parent)
    : QObject(parent)
{
}

ActionsHandler::~ActionsHandler(){};

NeoChatRoom *ActionsHandler::room() const
{
    return m_room;
}

void ActionsHandler::setRoom(NeoChatRoom *room)
{
    if (m_room == room) {
        return;
    }

    m_room = room;
    Q_EMIT roomChanged();
}

Connection *ActionsHandler::connection() const
{
    return m_connection;
}

void ActionsHandler::setConnection(Connection *connection)
{
    if (m_connection == connection) {
        return;
    }
    if (m_connection != nullptr) {
        disconnect(m_connection, &Connection::directChatAvailable, nullptr, nullptr);
    }
    m_connection = connection;
    if (m_connection != nullptr) {
        connect(m_connection, &Connection::directChatAvailable, this, [this](Quotient::Room *room) {
            room->setDisplayed(true);
            RoomManager::instance().enterRoom(qobject_cast<NeoChatRoom *>(room));
        });
    }
    Q_EMIT connectionChanged();
}

void ActionsHandler::postEdit(const QString &text)
{

    const auto localId = Controller::instance().activeConnection()->userId();
    for (auto it = m_room->messageEvents().crbegin(); it != m_room->messageEvents().crend(); ++it) {
        const auto &evt = **it;
        if (const auto event = eventCast<const RoomMessageEvent>(&evt)) {
            if (event->senderId() == localId && event->hasTextContent()) {
                static QRegularExpression re("^s/([^/]*)/([^/]*)");
                auto match = re.match(text);
                if (!match.hasMatch()) {
                    // should not happen but still make sure to send the message normally
                    // just in case.
                    postMessage(text, QString(), QString(), QString(), QVariantMap(), nullptr);
                }
                const QString regex = match.captured(1);
                const QString replacement = match.captured(2);
                QString originalString;
                if (event->content()) {
                    originalString = static_cast<const Quotient::EventContent::TextContent *>(event->content())->body;
                } else {
                    originalString = event->plainBody();
                }
                m_room->postHtmlMessage(text, originalString.replace(regex, replacement), event->msgtype(), "", event->id());
                return;
            }
        }
    }
}

void ActionsHandler::postMessage(const QString &text,
                                 const QString &attachementPath,
                                 const QString &replyEventId,
                                 const QString &editEventId,
                                 const QVariantMap &usernames,
                                 CustomEmojiModel* cem)
{
    QString rawText = text;
    QString cleanedText = text;

    auto preprocess = [cem](const QString& it) -> QString {
        if (cem == nullptr) {
            return it;
        }
        return cem->preprocessText(it);
    };

    for (auto it = usernames.constBegin(); it != usernames.constEnd(); it++) {
        cleanedText = cleanedText.replace(it.key(), "[" + it.key() + "](https://matrix.to/#/" + it.value().toString() + ")");
    }

    if (attachementPath.length() > 0) {
        m_room->uploadFile(attachementPath, cleanedText);
    }

    if (cleanedText.length() == 0) {
        return;
    }

    auto messageEventType = RoomMessageEvent::MsgType::Text;

    // Message commands
    static const QString shrugPrefix = QStringLiteral("/shrug");
    static const QString lennyPrefix = QStringLiteral("/lenny");
    static const QString tableflipPrefix = QStringLiteral("/tableflip");
    static const QString unflipPrefix = QStringLiteral("/unflip");
    static const QString plainPrefix = QStringLiteral("/plain "); // TODO
    static const QString htmlPrefix = QStringLiteral("/html "); // TODO
    static const QString rainbowPrefix = QStringLiteral("/rainbow ");
    static const QString rainbowmePrefix = QStringLiteral("/rainbowme ");
    static const QString spoilerPrefix = QStringLiteral("/spoiler ");
    static const QString mePrefix = QStringLiteral("/me ");
    static const QString noticePrefix = QStringLiteral("/notice ");

    // Actions commands
    static const QString ddgPrefix = QStringLiteral("/ddg "); // TODO
    static const QString nickPrefix = QStringLiteral("/nick "); // TODO
    static const QString meroomnickPrefix = QStringLiteral("/myroomnick "); // TODO
    static const QString roomavatarPrefix = QStringLiteral("/roomavatar "); // TODO
    static const QString myroomavatarPrefix = QStringLiteral("/myroomavatar "); // TODO
    static const QString myavatarPrefix = QStringLiteral("/myavatar "); // TODO
    static const QString invitePrefix = QStringLiteral("/invite ");
    static const QString joinPrefix = QStringLiteral("/join ");
    static const QString joinShortPrefix = QStringLiteral("/j ");
    static const QString partPrefix = QStringLiteral("/part");
    static const QString leavePrefix = QStringLiteral("/leave");
    static const QString ignorePrefix = QStringLiteral("/ignore ");
    static const QString unignorePrefix = QStringLiteral("/unignore ");
    static const QString queryPrefix = QStringLiteral("/query "); // TODO
    static const QString msgPrefix = QStringLiteral("/msg "); // TODO
    static const QString reactPrefix = QStringLiteral("/react ");

    // Admin commands

    static QStringList rainbowColors{"#ff2b00", "#ff5500", "#ff8000", "#ffaa00", "#ffd500", "#ffff00", "#d4ff00", "#aaff00", "#80ff00",
                                     "#55ff00", "#2bff00", "#00ff00", "#00ff2b", "#00ff55", "#00ff80", "#00ffaa", "#00ffd5", "#00ffff",
                                     "#00d4ff", "#00aaff", "#007fff", "#0055ff", "#002bff", "#0000ff", "#2a00ff", "#5500ff", "#7f00ff",
                                     "#aa00ff", "#d400ff", "#ff00ff", "#ff00d4", "#ff00aa", "#ff0080", "#ff0055", "#ff002b", "#ff0000"};

    if (cleanedText.indexOf(shrugPrefix) == 0) {
        cleanedText = QStringLiteral("¯\\_(ツ)_/¯") % cleanedText.remove(0, shrugPrefix.length());
        m_room->postHtmlMessage(cleanedText, cleanedText, messageEventType, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(lennyPrefix) == 0) {
        cleanedText = QStringLiteral("( ͡° ͜ʖ ͡°)") % cleanedText.remove(0, lennyPrefix.length());
        m_room->postHtmlMessage(cleanedText, cleanedText, messageEventType, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(tableflipPrefix) == 0) {
        cleanedText = QStringLiteral("(╯°□°）╯︵ ┻━┻") % cleanedText.remove(0, tableflipPrefix.length());
        m_room->postHtmlMessage(cleanedText, cleanedText, messageEventType, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(unflipPrefix) == 0) {
        cleanedText = QStringLiteral("┬──┬ ノ( ゜-゜ノ)") % cleanedText.remove(0, unflipPrefix.length());
        m_room->postHtmlMessage(cleanedText, cleanedText, messageEventType, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(rainbowPrefix) == 0) {
        cleanedText = cleanedText.remove(0, rainbowPrefix.length());
        QString rainbowText;
        for (int i = 0; i < cleanedText.length(); i++) {
            rainbowText = rainbowText % QStringLiteral("<font color='") % rainbowColors.at(i % rainbowColors.length()) % "'>" % cleanedText.at(i) % "</font>";
        }
        m_room->postHtmlMessage(cleanedText, preprocess(rainbowText), RoomMessageEvent::MsgType::Notice, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(spoilerPrefix) == 0) {
        cleanedText = cleanedText.remove(0, spoilerPrefix.length());
        const QStringList splittedText = rawText.split(" ");
        QString spoilerHtml = QStringLiteral("<span data-mx-spoiler>") % preprocess(cleanedText) % QStringLiteral("</span>");
        m_room->postHtmlMessage(cleanedText, spoilerHtml, RoomMessageEvent::MsgType::Notice, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(rainbowmePrefix) == 0) {
        cleanedText = cleanedText.remove(0, rainbowmePrefix.length());
        QString rainbowText;
        for (int i = 0; i < cleanedText.length(); i++) {
            rainbowText = rainbowText % QStringLiteral("<font color='") % rainbowColors.at(i % rainbowColors.length()) % "'>" % cleanedText.at(i) % "</font>";
        }
        m_room->postHtmlMessage(cleanedText, preprocess(rainbowText), messageEventType, replyEventId, editEventId);
        return;
    }

    if (rawText.indexOf(joinPrefix) == 0 || rawText.indexOf(joinShortPrefix) == 0) {
        if (rawText.indexOf(joinPrefix) == 0) {
            rawText = rawText.remove(0, joinPrefix.length());
        } else {
            rawText = rawText.remove(0, joinShortPrefix.length());
        }
        const QStringList splittedText = rawText.split(" ");
        if (text.count() == 0) {
            Q_EMIT showMessage(MessageType::Error, i18n("Invalid command"));
            return;
        }
        if (splittedText.count() > 1) {
            Controller::instance().joinRoom(splittedText[0] + ":" + splittedText[1]);
            return;
        } else if (splittedText[0].indexOf(":") != -1) {
            Controller::instance().joinRoom(splittedText[0]);
            return;
        } else {
            Controller::instance().joinRoom(splittedText[0] + ":matrix.org");
        }
        return;
    }

    if (rawText.indexOf(invitePrefix) == 0) {
        rawText = rawText.remove(0, invitePrefix.length());
        const QStringList splittedText = rawText.split(" ");
        if (splittedText.count() == 0) {
            Q_EMIT showMessage(MessageType::Error, i18n("Invalid command"));
            return;
        }
        m_room->inviteToRoom(splittedText[0]);
        return;
    }

    if (rawText.indexOf(partPrefix) == 0 || rawText.indexOf(leavePrefix) == 0) {
        if (rawText.indexOf(partPrefix) == 0) {
            rawText = rawText.remove(0, partPrefix.length());
        } else {
            rawText = rawText.remove(0, leavePrefix.length());
        }
        const QStringList splittedText = rawText.split(" ");
        if (splittedText.count() == 0 || splittedText[0].isEmpty()) {
            // leave current room
            m_connection->leaveRoom(m_room);
            return;
        }
        m_connection->leaveRoom(m_connection->room(splittedText[0]));
        return;
    }

    if (rawText.indexOf(ignorePrefix) == 0) {
        rawText = rawText.remove(0, ignorePrefix.length());
        const QStringList splittedText = rawText.split(" ");
        if (splittedText.count() == 0) {
            Q_EMIT showMessage(MessageType::Error, i18n("Invalid command"));
            return;
        }

        if (m_connection->users().contains(splittedText[0])) {
            Q_EMIT showMessage(MessageType::Error, i18n("Invalid command"));
            return;
        }

        const auto *user = m_connection->users()[splittedText[0]];
        m_connection->addToIgnoredUsers(user);
        return;
    }

    if (rawText.indexOf(unignorePrefix) == 0) {
        rawText = rawText.remove(0, unignorePrefix.length());
        const QStringList splittedText = rawText.split(" ");
        if (splittedText.count() == 0) {
            Q_EMIT showMessage(MessageType::Error, i18n("Invalid command"));
            return;
        }

        if (m_connection->users().contains(splittedText[0])) {
            Q_EMIT showMessage(MessageType::Error, i18n("Invalid command"));
            return;
        }

        const auto *user = m_connection->users()[splittedText[0]];
        m_connection->removeFromIgnoredUsers(user);
        return;
    }

    if (rawText.indexOf(reactPrefix) == 0) {
        rawText = rawText.remove(0, reactPrefix.length());
        if (replyEventId.isEmpty()) {
            for (auto it = m_room->messageEvents().crbegin(); it != m_room->messageEvents().crend(); ++it) {
                const auto &evt = **it;
                if (const auto event = eventCast<const RoomMessageEvent>(&evt)) {
                    m_room->toggleReaction(event->id(), rawText);
                    return;
                }
            }
            Q_EMIT showMessage(MessageType::Error, i18n("Couldn't find a message to react to"));
            return;
        }
        m_room->toggleReaction(replyEventId, rawText);
        return;
    }

    if (cleanedText.indexOf(mePrefix) == 0) {
        cleanedText = cleanedText.remove(0, mePrefix.length());
        messageEventType = RoomMessageEvent::MsgType::Emote;
        rawText = rawText.remove(0, mePrefix.length());
    } else if (cleanedText.indexOf(noticePrefix) == 0) {
        cleanedText = cleanedText.remove(0, noticePrefix.length());
        messageEventType = RoomMessageEvent::MsgType::Notice;
    }
    m_room->postMessage(rawText, preprocess(m_room->preprocessText(cleanedText)), messageEventType, replyEventId, editEventId);
}
