// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionshandler.h"
#include "commandmodel.h"

#include "csapi/joining.h"

#include <KLocalizedString>
#include <QDebug>
#include <QStringBuilder>

#include "controller.h"

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
            Q_EMIT Controller::instance().roomJoined(room->id());
        });
    }
    Q_EMIT connectionChanged();
}

void ActionsHandler::postMessage(const QString &text,
                                 const QString &attachementPath,
                                 const QString &replyEventId,
                                 const QString &editEventId,
                                 const QVariantMap &usernames)
{
    QString rawText = text;
    QString cleanedText = text;

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
    static const QString shrugPrefix = QStringLiteral("/shrug ");
    static const QString lennyPrefix = QStringLiteral("/lenny ");
    static const QString plainPrefix = QStringLiteral("/plain "); // TODO
    static const QString htmlPrefix = QStringLiteral("/html "); // TODO
    static const QString rainbowPrefix = QStringLiteral("/rainbow ");
    static const QString rainbowmePrefix = QStringLiteral("/rainbowme ");
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
    static const QString partPrefix = QStringLiteral("/part");
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
        cleanedText = QStringLiteral("¯\\\\_(ツ)\\_/¯") % cleanedText.remove(0, shrugPrefix.length());
        m_room->postHtmlMessage(cleanedText, cleanedText, messageEventType, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(lennyPrefix) == 0) {
        cleanedText = QStringLiteral("( ͡° ͜ʖ ͡°)") % cleanedText.remove(0, lennyPrefix.length());
        m_room->postHtmlMessage(cleanedText, cleanedText, messageEventType, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(rainbowPrefix) == 0) {
        cleanedText = cleanedText.remove(0, rainbowPrefix.length());
        QString rainbowText;
        for (int i = 0; i < cleanedText.length(); i++) {
            rainbowText = rainbowText % QStringLiteral("<font color='") % rainbowColors.at(i % rainbowColors.length()) % "'>" % cleanedText.at(i) % "</font>";
        }
        m_room->postHtmlMessage(cleanedText, rainbowText, RoomMessageEvent::MsgType::Notice, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(rainbowmePrefix) == 0) {
        cleanedText = cleanedText.remove(0, rainbowmePrefix.length());
        QString rainbowText;
        for (int i = 0; i < cleanedText.length(); i++) {
            rainbowText = rainbowText % QStringLiteral("<font color='") % rainbowColors.at(i % rainbowColors.length()) % "'>" % cleanedText.at(i) % "</font>";
        }
        m_room->postHtmlMessage(cleanedText, rainbowText, messageEventType, replyEventId, editEventId);
        return;
    }

    if (rawText.indexOf(joinPrefix) == 0) {
        rawText = rawText.remove(0, joinPrefix.length());
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

    if (rawText.indexOf(partPrefix) == 0) {
        rawText = rawText.remove(0, partPrefix.length());
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
        if (replyEventId.isEmpty()) {
            return;
        }
        rawText = rawText.remove(0, reactPrefix.length());
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
    m_room->postMessage(rawText, cleanedText, messageEventType, replyEventId, editEventId);
}
