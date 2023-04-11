// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <KConfig>
#include <KConfigGroup>
#include <QObject>
#include <uriresolver.h>

#include "chatdocumenthandler.h"

class NeoChatRoom;

namespace Quotient
{
class Room;
class User;
}

using namespace Quotient;

class RoomManager : public QObject, public UriResolverBase
{
    Q_OBJECT

    /// This property holds the current open room in the NeoChat, if any.
    /// \sa hasOpenRoom
    Q_PROPERTY(NeoChatRoom *currentRoom READ currentRoom NOTIFY currentRoomChanged)

    /// This property holds whether a room is currently open in NeoChat.
    /// \sa room
    Q_PROPERTY(bool hasOpenRoom READ hasOpenRoom NOTIFY currentRoomChanged)
    Q_PROPERTY(ChatDocumentHandler *chatDocumentHandler READ chatDocumentHandler WRITE setChatDocumentHandler NOTIFY chatDocumentHandlerChanged)

public:
    explicit RoomManager(QObject *parent = nullptr);
    virtual ~RoomManager();
    static RoomManager &instance();

    /// Load the last opened room or the welcome page.
    Q_INVOKABLE void loadInitialRoom();

    /// This method will tell the NeoChat to open the message list
    /// with the given room.
    Q_INVOKABLE void enterRoom(NeoChatRoom *room);

    Q_INVOKABLE void openWindow(NeoChatRoom *room);

    /// Leave the room and close it if it is open.
    Q_INVOKABLE void leaveRoom(NeoChatRoom *room);

    /// Getter for the currentRoom property.
    NeoChatRoom *currentRoom() const;

    /// Getter for the hasOpenRoom property.
    bool hasOpenRoom() const;

    // Overrided methods from UriResolverBase
    UriResolveResult visitUser(User *user, const QString &action) override;
    void joinRoom(Quotient::Connection *account, const QString &roomAliasOrId, const QStringList &viaServers) override;
    // TODO: it need also override in the feature?
#ifdef QUOTIENT_07
    void knockRoom(Quotient::Connection *account, const QString &roomAliasOrId, const QString &reason, const QStringList &viaServers);
#endif
    Q_INVOKABLE void visitRoom(Quotient::Room *room, const QString &eventId) override;
    Q_INVOKABLE bool visitNonMatrix(const QUrl &url) override;

    Q_INVOKABLE void openResource(const QString &idOrUri, const QString &action = {});

    /// Call this when the current used connection is dropped.
    Q_INVOKABLE void reset();

    ChatDocumentHandler *chatDocumentHandler() const;
    void setChatDocumentHandler(ChatDocumentHandler *handler);

    void setUrlArgument(const QString &arg);

Q_SIGNALS:
    /// Signal triggered when the current open room change.
    void currentRoomChanged();

    /// Signal triggered when the pageStack should push a new page with the
    /// message list for the given room.
    void pushRoom(NeoChatRoom *room, const QString &event);

    /// Signal triggered when the room displayed by the message list should
    /// be changed.
    void replaceRoom(NeoChatRoom *room, const QString &event);

    /// Go to the specified event in the current room.
    void goToEvent(const QString &event);

    /// Signal triggered when a room need to be opened in a new window.
    void openRoomInNewWindow(NeoChatRoom *room);

    /// Ask current room to open the user's details for the give user.
    /// This can assume the user is loaded.
    void showUserDetail(const Quotient::User *user);

    /// Ask current room to show confirmation dialog to open direct chat.
    /// This can assume the user is loaded.
    void askDirectChatConfirmation(const Quotient::User *user);

    /// Displays warning to the user.
    void warning(const QString &title, const QString &message);

    void chatDocumentHandlerChanged();

private:
    void openRoomForActiveConnection();

    NeoChatRoom *m_currentRoom;
    NeoChatRoom *m_lastCurrentRoom;
    QString m_arg;
    KConfig m_config;
    KConfigGroup m_lastRoomConfig;
    ChatDocumentHandler *m_chatDocumentHandler;
};
