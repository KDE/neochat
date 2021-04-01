// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: GPl-3.0-or-later

#include "actionshandler.h"

#include "csapi/joining.h"

#include <KLocalizedString>
#include <QStringBuilder>
#include <QDebug>

ActionsHandler::ActionsHandler(QObject *parent)
    : QObject(parent)
{
}

ActionsHandler::~ActionsHandler()
{};

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
        connect(m_connection, &Connection::directChatAvailable,
                this, [this](Quotient::Room *room) {
                    room->setDisplayed(true);
                    Q_EMIT roomJoined(room->id());
                });
    }
    Q_EMIT connectionChanged();
}

QVariantList ActionsHandler::commands() const
{
    QVariantList commands;
    // Messages commands
    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/shrug "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "<message>"),
        QStringLiteral("help"), i18n("Prepends ¯\\_(ツ)_/¯ to a plain-text message")
    });

    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/lenny "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "<message>"),
        QStringLiteral("help"), i18n("Prepends ( ͡° ͜ʖ ͡°) to a plain-text message")
    });

    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/plain "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "<message>"),
        QStringLiteral("help"), i18n("Sends a message as plain text, without interpreting it as markdown")
    });

    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/html "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "<message>"),
        QStringLiteral("help"), i18n("Sends a message as html, without interpreting it as markdown")
    });

    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/rainbow "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "<message>"),
        QStringLiteral("help"), i18n("Sends the given message coloured as a rainbow")
    });

    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/rainbowme "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "<message>"),
        QStringLiteral("help"), i18n("Sends the given emote coloured as a rainbow")
    });

    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/me "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "<message>"),
        QStringLiteral("help"), i18n("Displays action")
    });

    // Actions commands
    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/join "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "<room-address>"),
        QStringLiteral("help"), i18n("Joins room with given address")
    });

    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/part "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "[<room-address>]"),
        QStringLiteral("help"), i18n("Leave room")
    });

    commands.append({
        QStringLiteral("prefix"), QStringLiteral("/invite "),
        QStringLiteral("parameter"), i18nc("@label Parameter of a command", "<user-id>"),
        QStringLiteral("help"), i18n("Invites user with given id to current room")
    });

    // TODO more see elements /help action

    return commands;
}

void ActionsHandler::joinRoom(const QString &alias)
{
    if (!alias.contains(":")) {
        Q_EMIT showMessage(MessageType::Error, i18n("The room id you are trying to join is not valid"));
        return;
    }

    const auto knownServer = alias.mid(alias.indexOf(":") + 1);
    auto joinRoomJob = m_connection->joinRoom(alias, QStringList{knownServer});

    Quotient::JoinRoomJob::connect(joinRoomJob, &JoinRoomJob::failure, [=] {
        Q_EMIT showMessage(MessageType::Error, i18n("Server error when joining the room \"%1\": %2",
                    joinRoomJob->errorString()));
    });
    Quotient::JoinRoomJob::connect(joinRoomJob, &JoinRoomJob::success, [this, joinRoomJob] {
        Q_EMIT roomJoined(joinRoomJob->roomId());
    });
}

void ActionsHandler::createRoom(const QString &name, const QString &topic)
{
    auto createRoomJob = m_connection->createRoom(Connection::PublishRoom, "", name, topic, QStringList());
    Quotient::CreateRoomJob::connect(createRoomJob, &CreateRoomJob::failure, [=] {
        Q_EMIT showMessage(MessageType::Error, i18n("Room creation failed: \"%1\"", createRoomJob->errorString()));
    });
    Quotient::CreateRoomJob::connect(createRoomJob, &CreateRoomJob::success, [=] {
        Q_EMIT roomJoined(createRoomJob->roomId());
    });
}

void ActionsHandler::postMessage(const QString &text,
        const QString &attachementPath, const QString &replyEventId, const QString &editEventId,
        const QVariantMap &usernames)
{
    QString rawText = text;
    QString cleanedText = text;


    for (auto it = usernames.constBegin(); it != usernames.constEnd(); it++) {
        cleanedText = cleanedText.replace(it.key(),
                "[" + it.key() + "](https://matrix.to/#/" + it.value().toString() + ")");
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

    // Admin commands 

    static QStringList rainbowColors{"#ff2b00", "#ff5500", "#ff8000", "#ffaa00", "#ffd500",
        "#ffff00", "#d4ff00", "#aaff00", "#80ff00", "#55ff00", "#2bff00", "#00ff00", "#00ff2b",
        "#00ff55", "#00ff80", "#00ffaa", "#00ffd5", "#00ffff", "#00d4ff", "#00aaff", "#007fff",
        "#0055ff", "#002bff", "#0000ff", "#2a00ff", "#5500ff", "#7f00ff", "#aa00ff", "#d400ff",
        "#ff00ff", "#ff00d4", "#ff00aa", "#ff0080", "#ff0055", "#ff002b", "#ff0000"};

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
        m_room->postHtmlMessage(cleanedText, rainbowText,  RoomMessageEvent::MsgType::Notice, replyEventId, editEventId);
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
            joinRoom(splittedText[0] + ":" + splittedText[1]);
            return;
        } else if (splittedText[0].indexOf(":") != -1) {
            joinRoom(splittedText[0]);
            return;
        }
        else {
            joinRoom(splittedText[0] + ":matrix.org");
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

    if (cleanedText.indexOf(mePrefix) == 0) {
        cleanedText = cleanedText.remove(0, mePrefix.length());
        messageEventType = RoomMessageEvent::MsgType::Emote;
    } else if (cleanedText.indexOf(noticePrefix) == 0) {
        cleanedText = cleanedText.remove(0, noticePrefix.length());
        messageEventType = RoomMessageEvent::MsgType::Notice;
    }
    m_room->postMessage(rawText, cleanedText, messageEventType, replyEventId, editEventId);
}
