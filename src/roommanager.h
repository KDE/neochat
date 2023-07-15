// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <KConfig>
#include <KConfigGroup>
#include <QObject>
#include <Quotient/uriresolver.h>

#include "chatdocumenthandler.h"

class NeoChatRoom;

namespace Quotient
{
class Room;
class User;
}

using namespace Quotient;

/**
 * @class RoomManager
 *
 * A singleton class to help manage which room is open in NeoChat.
 */
class RoomManager : public QObject, public UriResolverBase
{
    Q_OBJECT

    /**
     * @brief The current open room in NeoChat, if any.
     *
     * @sa hasOpenRoom
     */
    Q_PROPERTY(NeoChatRoom *currentRoom READ currentRoom NOTIFY currentRoomChanged)

    /**
     * @brief Whether a room is currently open in NeoChat.
     *
     * @sa room
     */
    Q_PROPERTY(bool hasOpenRoom READ hasOpenRoom NOTIFY currentRoomChanged)

    /**
     * @brief The ChatDocumentHandler for the open room.
     *
     * @sa ChatDocumentHandler
     */
    Q_PROPERTY(ChatDocumentHandler *chatDocumentHandler READ chatDocumentHandler WRITE setChatDocumentHandler NOTIFY chatDocumentHandlerChanged)

public:
    explicit RoomManager(QObject *parent = nullptr);
    virtual ~RoomManager();
    static RoomManager &instance();

    NeoChatRoom *currentRoom() const;

    bool hasOpenRoom() const;

    /**
     * @brief Load the last opened room or the welcome page.
     */
    Q_INVOKABLE void loadInitialRoom();

    /**
     * @brief Enter the given room.
     *
     * This method will tell NeoChat to open the message list with the given room.
     */
    Q_INVOKABLE void enterRoom(NeoChatRoom *room);

    /**
     * @brief Open a new window with the given room.
     *
     * The open window will have its own message list for the given room.
     */
    Q_INVOKABLE void openWindow(NeoChatRoom *room);

    /**
     * @brief Leave the room and close it if it is open.
     */
    Q_INVOKABLE void leaveRoom(NeoChatRoom *room);

    // Overrided methods from UriResolverBase
    /**
     * @brief Resolve a user URI.
     *
     * This overloads Quotient::UriResolverBase::visitUser().
     *
     * Called by Quotient::UriResolverBase::visitResource() when the passed URI
     * identifies a Matrix user.
     *
     * @sa Quotient::UriResolverBase::visitUser(), Quotient::UriResolverBase::visitResource()
     */
    UriResolveResult visitUser(User *user, const QString &action) override;

    /**
     * @brief Visit a room.
     *
     * This overloads Quotient::UriResolverBase::visitRoom().
     *
     * Called by Quotient::UriResolverBase::visitResource() when the passed URI
     * identifies a room or an event in a room.
     *
     * @sa Quotient::UriResolverBase::visitRoom(), Quotient::UriResolverBase::visitResource()
     */
    Q_INVOKABLE void visitRoom(Quotient::Room *room, const QString &eventId) override;

    /**
     * @brief Join a room.
     *
     * This overloads Quotient::UriResolverBase::joinRoom().
     *
     * Called by Quotient::UriResolverBase::visitResource() when the passed URI has
     * `action() == "join"` and identifies a room that the user defined by the
     * Connection argument is not a member of.
     *
     * @sa Quotient::UriResolverBase::joinRoom(), Quotient::UriResolverBase::visitResource()
     */
    void joinRoom(Quotient::Connection *account, const QString &roomAliasOrId, const QStringList &viaServers) override;

    /**
     * @brief Visit a non-matrix resource.
     *
     * This overloads Quotient::UriResolverBase::visitNonMatrix().
     *
     * Called by Quotient::UriResolverBase::visitResource() when the passed URI
     * has `type() == NonMatrix`
     *
     * @sa Quotient::UriResolverBase::visitNonMatrix(), Quotient::UriResolverBase::visitResource()
     */
    Q_INVOKABLE bool visitNonMatrix(const QUrl &url) override;

    /**
     * @brief Knock a room.
     *
     * See https://spec.matrix.org/latest/client-server-api/#knocking-on-rooms for
     * knocking on rooms.
     */
    void knockRoom(Quotient::Connection *account, const QString &roomAliasOrId, const QString &reason, const QStringList &viaServers);

    /**
     * @brief Open the given resource.
     *
     * Convenience function to call Quotient::UriResolverBase::visitResource() from
     * QML if valid.
     *
     * @sa Quotient::UriResolverBase::visitResource()
     */
    Q_INVOKABLE void openResource(const QString &idOrUri, const QString &action = {});

    /**
     * @brief Call this when the current used connection is dropped.
     */
    Q_INVOKABLE void reset();

    ChatDocumentHandler *chatDocumentHandler() const;
    void setChatDocumentHandler(ChatDocumentHandler *handler);

    /**
     * @brief Set a URL to be loaded as the initial room.
     */
    void setUrlArgument(const QString &arg);

Q_SIGNALS:
    void currentRoomChanged();

    /**
     * @brief Push a new room page.
     *
     * Signal triggered when the main window pageStack should push a new page with
     * the message list for the given room.
     *
     * @param room the room to be shown on the new page.
     * @param event the event to got to if available.
     */
    void pushRoom(NeoChatRoom *room, const QString &event);

    /**
     * @brief Replace the existing room.
     *
     * Signal triggered when the room displayed by the message list should be changed.
     *
     * @param room the room to be shown on the new page.
     * @param event the event to got to if available.
     */
    void replaceRoom(NeoChatRoom *room, const QString &event);

    /**
     * @brief Go to the specified event in the current room.
     */
    void goToEvent(const QString &event);

    /**
     * @brief Open room in a new window.
     *
     * Signal triggered when a room needs to be opened in a new window.
     */
    void openRoomInNewWindow(NeoChatRoom *room);

    /**
     * @brief Show details for the given user.
     *
     * Ask current room to open the user's details for the give user.
     * This assumes the user is loaded.
     */
    void showUserDetail(const Quotient::User *user);

    /**
     * @brief Show the direct chat confirmation dialog.
     *
     * Ask current room to show confirmation dialog to open direct chat.
     * This assumes the user is loaded.
     */
    void askDirectChatConfirmation(const Quotient::User *user);

    /**
     * @brief Displays warning to the user.
     */
    void warning(const QString &title, const QString &message);

    void chatDocumentHandlerChanged();

private:
    void openRoomForActiveConnection();

    NeoChatRoom *m_currentRoom;
    NeoChatRoom *m_lastCurrentRoom;
    QString m_arg;
    KConfig m_config;
    KConfigGroup m_lastRoomConfig;
    QPointer<ChatDocumentHandler> m_chatDocumentHandler;
};
