// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2021 Alexey Rusakov <TODO>
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roommanager.h"
#include "neochatroom.h"
#include "neochatconfig.h"
#include "controller.h"
#include <QDesktopServices>
#include <KLocalizedString>
#include <csapi/joining.h>
#include <utility>

RoomManager::RoomManager(QObject *parent)
    : QObject(parent)
    , m_currentRoom(nullptr)
    , m_lastCurrentRoom(nullptr)
{}

RoomManager::~RoomManager()
{}

NeoChatRoom *RoomManager::currentRoom() const
{
    return m_currentRoom;
}

void RoomManager::openResource(const QString &idOrUri, const QString &action)
{
    Uri uri { idOrUri };
    if (!uri.isValid()) {
        Q_EMIT warning(i18n("Malformed or empty Matrix id"),
            i18n("%1 is not a correct Matrix identifier", idOrUri));
        return;
    }
    auto account = Controller::instance().activeConnection();

    if (uri.type() != Uri::NonMatrix) {
        if (!account) {
            return;
        }
        if (!action.isEmpty()) {
            uri.setAction(action);
        }
        // TODO we should allow the user to select a connection.
    }

    const auto result = visitResource(account, uri);
    if (result == Quotient::CouldNotResolve) {
        Q_EMIT warning(i18n("Room not found"),
                       i18n("There's no room %1 in the room list. Check the spelling and the account.", idOrUri));
    } else { // Invalid cases should have been eliminated earlier
        Q_ASSERT(result == Quotient::UriResolved);
    }
}

bool RoomManager::hasOpenRoom() const
{
    return m_currentRoom != nullptr;
}

void RoomManager::setUrlArgument(const QString &arg)
{
    m_arg = arg;
}

void RoomManager::loadInitialRoom()
{
    Q_ASSERT(Controller::instance().activeConnection());

    if (!m_arg.isEmpty()) {
        openResource(m_arg);
    }

    if (m_currentRoom) {
        // we opened a room with the arg parsing already
        return;
    }

    if (!NeoChatConfig::self()->openRoom().isEmpty()) {
        // Here we can cast because the controller has been configured to
        // return NeoChatRoom instead of simple Quotient::Room
        const auto room = qobject_cast<NeoChatRoom *>(
                Controller::instance().activeConnection()->room(NeoChatConfig::self()->openRoom()));
        m_lastCurrentRoom = std::exchange(m_currentRoom, room);
        Q_EMIT currentRoomChanged();
        Q_EMIT pushRoom(room, QString());
    } else {
        Q_EMIT pushWelcomePage();
    }
}

void RoomManager::enterRoom(NeoChatRoom *room)
{
    if (!m_currentRoom) {
        m_lastCurrentRoom = std::exchange(m_currentRoom, room);
        Q_EMIT currentRoomChanged();
        Q_EMIT pushRoom(room, QString());
    }

    m_lastCurrentRoom = std::exchange(m_currentRoom, room);
    Q_EMIT currentRoomChanged();
    Q_EMIT replaceRoom(m_currentRoom, QString());

    NeoChatConfig::self()->setOpenRoom(room->id());
    NeoChatConfig::self()->save();
}

void RoomManager::openWindow(NeoChatRoom *room)
{
    // forward the call to QML
    Q_EMIT openRoomInNewWindow(room);
}

UriResolveResult RoomManager::visitUser(User* user, const QString &action)
{
    if (action == "mention" || action.isEmpty()) {
        // send it has QVariantMap because the properties in the
#ifdef QUOTIENT_07
        user->load();
#endif
        Q_EMIT showUserDetail(user);
    } else if (action == "_interactive") {
        user->requestDirectChat();
    } else if (action == "chat") {
#ifdef QUOTIENT_07
        user->load();
#endif
        Q_EMIT askDirectChatConfirmation(user);
    } else {
        return Quotient::IncorrectAction;
    }

    return Quotient::UriResolved;
}

void RoomManager::visitRoom(Room *room, const QString &eventId)
{
    auto neoChatRoom = qobject_cast<NeoChatRoom *>(room);
    Q_ASSERT(neoChatRoom != nullptr);

    if (m_currentRoom) {
        if (m_currentRoom->id() == room->id()) {
            Q_EMIT goToEvent(eventId);
        } else {
            m_lastCurrentRoom = std::exchange(m_currentRoom, neoChatRoom);
            Q_EMIT currentRoomChanged();
            Q_EMIT replaceRoom(neoChatRoom, eventId);
        }
    } else {
        m_lastCurrentRoom = std::exchange(m_currentRoom, neoChatRoom);
        Q_EMIT currentRoomChanged();
        Q_EMIT pushRoom(neoChatRoom, eventId);
    }
}

void RoomManager::joinRoom(Quotient::Connection *account,
                          const QString &roomAliasOrId,
                          const QStringList &viaServers)
{
    // We already listen to roomJoined signal in the Controller
    account->joinRoom(QUrl::toPercentEncoding(roomAliasOrId), viaServers);
}

bool RoomManager::visitNonMatrix(const QUrl &url)
{
    // Return true if the user cancels, treating it as an alternative normal
    // flow (rather than an abnormal flow when the navigation itself fails).
    if (NeoChatConfig::self()->confirmLinksAction()) {
        Q_EMIT openLink(url);
    } else {
        if (!QDesktopServices::openUrl(url)) {
            Q_EMIT warning(i18n("No application for the link"),
                           i18n("Your operating system could not find an application for the link."));
        }
    }
    return true;
}

void RoomManager::reset()
{
    m_arg = QString();
    m_currentRoom = nullptr;
    m_lastCurrentRoom = nullptr;
    Q_EMIT currentRoomChanged();
}

void RoomManager::leaveRoom(NeoChatRoom *room)
{
    // close the room and open the last open room if it exist
    // other open welcome page.
    if (m_currentRoom && m_currentRoom->id() == room->id()) {
        if (!m_lastCurrentRoom) {
            Q_EMIT pushWelcomePage();
            return;
        }
        m_currentRoom = m_lastCurrentRoom;
        m_lastCurrentRoom = nullptr;

        Q_EMIT currentRoomChanged();
        Q_EMIT replaceRoom(m_lastCurrentRoom, QString());
    }

    room->forget();
}
