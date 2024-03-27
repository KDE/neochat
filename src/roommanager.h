// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <KConfigGroup>
#include <KSharedConfig>
#include <QObject>
#include <QQmlEngine>
#include <Quotient/room.h>
#include <Quotient/uriresolver.h>
#include <KConfigGroup>

#include "chatdocumenthandler.h"
#include "enums/messagecomponenttype.h"
#include "eventhandler.h"
#include "models/mediamessagefiltermodel.h"
#include "models/messagefiltermodel.h"
#include "models/timelinemodel.h"

class NeoChatRoom;
class NeoChatConnection;

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
 *
 * This class also inherits UriResolverBase and overrides the relevant functions to
 * resolve various URIs. The base functions visitUser(), visitRoom(), etc are held
 * private intentionally and instead resolveResource() should be called with either
 * an appropriate URI or a Matrix ID and action.
 */
class RoomManager : public QObject, public UriResolverBase
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The current open room in NeoChat, if any.
     *
     * @sa hasOpenRoom
     */
    Q_PROPERTY(NeoChatRoom *currentRoom READ currentRoom NOTIFY currentRoomChanged)

    /**
     * @brief The TimelineModel that should be used for room message visualisation.
     *
     * The room object the model uses to get the data will be updated by this class
     * so there is no need to do this manually or replace the model when a room
     * changes.
     *
     * @note Available here so that the room page and drawer both have access to the
     *       same model.
     */
    Q_PROPERTY(TimelineModel *timelineModel READ timelineModel CONSTANT)

    /**
     * @brief The MessageFilterModel that should be used for room message visualisation.
     *
     * @note Available here so that the room page and drawer both have access to the
     *       same model.
     */
    Q_PROPERTY(MessageFilterModel *messageFilterModel READ messageFilterModel CONSTANT)

    /**
     * @brief The MediaMessageFilterModel that should be used for room media message visualisation.
     *
     * @note Available here so that the room page and drawer both have access to the
     *       same model.
     */
    Q_PROPERTY(MediaMessageFilterModel *mediaMessageFilterModel READ mediaMessageFilterModel CONSTANT)

    /**
     * @brief Whether a room is currently open in NeoChat.
     *
     * @sa room
     */
    Q_PROPERTY(bool hasOpenRoom READ hasOpenRoom NOTIFY currentRoomChanged)

    /**
     * @brief The room ID of the last space entered.
     */
    Q_PROPERTY(QString lastSpaceId READ lastSpaceId WRITE setLastSpaceId NOTIFY directChatsActiveChanged)

    /**
     * @brief Whether the last SpaceDrawer category selected was direct chats.
     */
    Q_PROPERTY(bool directChatsActive READ directChatsActive WRITE setDirectChatsActive NOTIFY directChatsActiveChanged)

    /**
     * @brief The ChatDocumentHandler for the open room.
     *
     * @sa ChatDocumentHandler
     */
    Q_PROPERTY(ChatDocumentHandler *chatDocumentHandler READ chatDocumentHandler WRITE setChatDocumentHandler NOTIFY chatDocumentHandlerChanged)

public:
    virtual ~RoomManager();
    static RoomManager &instance();
    static RoomManager *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    NeoChatRoom *currentRoom() const;

    TimelineModel *timelineModel() const;
    MessageFilterModel *messageFilterModel() const;
    MediaMessageFilterModel *mediaMessageFilterModel() const;

    /**
     * @brief Resolve the given URI resource.
     *
     * @note It's actually Quotient::UriResolverBase::visitResource() but with Q_INVOKABLE
     *       and the connection grabbed from RoomManager.
     *
     * @sa Quotient::UriResolverBase::visitResource()
     */
    Q_INVOKABLE UriResolveResult resolveResource(const Uri &uri);

    /**
     * @brief Resolve the given resource.
     *
     * @note It's actually Quotient::UriResolverBase::visitResource() but with Q_INVOKABLE
     *       and the connection grabbed from RoomManager.
     *
     * @sa Quotient::UriResolverBase::visitResource()
     */
    Q_INVOKABLE void resolveResource(const QString &idOrUri, const QString &action = {});

    bool hasOpenRoom() const;

    /**
     * @brief Load the last opened room or the welcome page.
     */
    Q_INVOKABLE void loadInitialRoom();

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

    /**
     * @brief Knock a room.
     *
     * See https://spec.matrix.org/latest/client-server-api/#knocking-on-rooms for
     * knocking on rooms.
     */
    void knockRoom(Quotient::Connection *account, const QString &roomAliasOrId, const QString &reason, const QStringList &viaServers);

    /**
     * @brief Show a media item maximized.
     *
     * @param index the index to open the maximize delegate model at. This is the
     *        index in the MediaMessageFilterModel owned by this RoomManager. A value
     *        of -1 opens a the default item.
     */
    Q_INVOKABLE void maximizeMedia(int index);

    /**
     * @brief Request that any full screen overlay currently open closes.
     */
    Q_INVOKABLE void requestFullScreenClose();

    /**
     * @brief Show the JSON source for the given event Matrix ID
     */
    Q_INVOKABLE void viewEventSource(const QString &eventId);

    /**
     * @brief Show a context menu for the given event.
     */
    Q_INVOKABLE void viewEventMenu(const QString &eventId, NeoChatRoom *room, const QString &selectedText = {});

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

    QString lastSpaceId() const;
    void setLastSpaceId(const QString &lastSpaceId);

    bool directChatsActive() const;
    void setDirectChatsActive(bool directChatsActive);

    NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

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
     * @brief Push a new space home page.
     *
     * Signal triggered when the main window pageStack should push a new page with
     * the space home for the given space room.
     *
     * @param spaceRoom the space room to be shown on the new page.
     */
    void pushSpaceHome(NeoChatRoom *spaceRoom);

    /**
     * @brief Replace the existing space home.
     *
     * Signal triggered when the currently displayed room page should be changed
     * to the space home for the given space room.
     *
     * @param spaceRoom the space room to be shown on the new page.
     */
    void replaceSpaceHome(NeoChatRoom *spaceRoom);

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
     * @brief Request a media item is shown maximized.
     *
     * @param index the index to open the maximize delegate model at. This is the
     *        index in the MediaMessageFilterModel owned by this RoomManager. A value
     *        of -1 opens a the default item.
     */
    void showMaximizedMedia(int index);

    /**
     * @brief Request that any full screen overlay closes.
     */
    void closeFullScreen();

    /**
     * @brief Request the JSON source for the given event ID is shown.
     */
    void showEventSource(const QString &eventId);

    /**
     * @brief Request to show a menu for the given event.
     */
    void showMessageMenu(const QString &eventId,
                         const QVariantMap &author,
                         MessageComponentType::Type messageComponentType,
                         const QString &plainText,
                         const QString &htmlText,
                         const QString &selectedText);

    /**
     * @brief Request to show a menu for the given media event.
     */
    void showFileMenu(const QString &eventId,
                      const QVariantMap &author,
                      MessageComponentType::Type messageComponentType,
                      const QString &plainText,
                      const QString &mimeType,
                      const FileTransferInfo &progressInfo);

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

    void connectionChanged();

    void directChatsActiveChanged();
    void lastSpaceIdChanged();

private:
    void openRoomForActiveConnection();

    QPointer<NeoChatRoom> m_currentRoom;
    NeoChatRoom *m_lastCurrentRoom;
    QString m_arg;
    KSharedConfig::Ptr m_config;
    KConfigGroup m_lastRoomConfig;
    KConfigGroup m_lastSpaceConfig;
    KConfigGroup m_directChatsConfig;
    QPointer<ChatDocumentHandler> m_chatDocumentHandler;

    TimelineModel *m_timelineModel;
    MessageFilterModel *m_messageFilterModel;
    MediaMessageFilterModel *m_mediaMessageFilterModel;
    NeoChatConnection *m_connection;

    /**
     * @brief Resolve a user URI.
     *
     * This overloads Quotient::UriResolverBase::visitUser().
     *
     * Called by Quotient::UriResolverBase::visitResource() when the passed URI
     * identifies a Matrix user.
     *
     * @note This is private as resolveResource() should always be called, which
     *       will in turn call Quotient::UriResolverBase::visitResource() and this
     *       function if appropriate for the URI.
     *
     * @sa resolveResource(), Quotient::UriResolverBase::visitUser(), Quotient::UriResolverBase::visitResource()
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
     * @note This is private as resolveResource() should always be called, which
     *       will in turn call Quotient::UriResolverBase::visitResource() and this
     *       function if appropriate for the URI.
     *
     * @sa resolveResource(), Quotient::UriResolverBase::visitUser(), Quotient::UriResolverBase::visitResource()
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
     * @note This is private as resolveResource() should always be called, which
     *       will in turn call Quotient::UriResolverBase::visitResource() and this
     *       function if appropriate for the URI.
     *
     * @sa resolveResource(), Quotient::UriResolverBase::visitUser(), Quotient::UriResolverBase::visitResource()
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
     * @note This is private as resolveResource() should always be called, which
     *       will in turn call Quotient::UriResolverBase::visitResource() and this
     *       function if appropriate for the URI.
     *
     * @sa resolveResource(), Quotient::UriResolverBase::visitUser(), Quotient::UriResolverBase::visitResource()
     */
    Q_INVOKABLE bool visitNonMatrix(const QUrl &url) override;

private:
    explicit RoomManager(QObject *parent = nullptr);
};
