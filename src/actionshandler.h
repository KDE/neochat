// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: GPl-3.0-or-later

#pragma once

#include <QObject>

#include "connection.h"
#include "neochatroom.h"

using namespace Quotient;

/// \brief Handles user interactions with NeoChat (joining room, creating room,
/// sending message). Account management is handled by Controller.
class ActionsHandler : public QObject
{
    Q_OBJECT

    /// \brief List of command definition. Useful for building an autocompletion
    /// engine or an help dialog.
    Q_PROPERTY(QVariantList commands READ commands CONSTANT)

    /// \brief The connection that will handle sending the message.
    Q_PROPERTY(Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /// \brief The connection that will handle sending the message.
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    enum MessageType {
        Info,
        Error,
    };
    Q_ENUM(MessageType);

    explicit ActionsHandler(QObject *parent = nullptr);
    ~ActionsHandler();

    QVariantList commands() const;

    [[nodiscard]] Connection *connection() const;
    void setConnection(Connection *connection);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

Q_SIGNALS:
    /// \brief Show error or information message.
    ///
    /// These messages will be displayed in the room view header.
    void showMessage(MessageType messageType, QString message);

    /// \brief Emitted when an action made the user join a room.
    ///
    /// Either when a new room was created, a direct chat was started
    /// or a group chat was joined. The UI will react to this signal
    /// and switch to the newly joined room.
    void roomJoined(QString roomName);

    void roomChanged();
    void connectionChanged();

public Q_SLOTS:
    /// \brief Create new room for a group chat.
    void createRoom(const QString &name, const QString &topic);

    /// \brief Join a room.
    void joinRoom(const QString &alias);

    /// \brief Post a message.
    ///
    /// This also interprets commands if any.
    void
    postMessage(const QString &text, const QString &attachementPath, const QString &replyEventId, const QString &editEventId, const QVariantMap &usernames);

private:
    Connection *m_connection = nullptr;
    NeoChatRoom *m_room = nullptr;
};
