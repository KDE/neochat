// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

namespace Quotient
{
class Connection;
}

class NeoChatRoom;
class CustomEmojiModel;

/// \brief Handles user interactions with NeoChat (joining room, creating room,
/// sending message). Account management is handled by Controller.
class ActionsHandler : public QObject
{
    Q_OBJECT

    /// \brief The connection that will handle sending the message.
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

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

    [[nodiscard]] Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *connection);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

Q_SIGNALS:
    /// \brief Show error or information message.
    ///
    /// These messages will be displayed in the room view header.
    void showMessage(ActionsHandler::MessageType messageType, QString message);

    void roomChanged();
    void connectionChanged();

public Q_SLOTS:

    /// \brief Post a message.
    ///
    /// This also interprets commands if any.
    void postMessage(const QString &text,
                     const QString &attachementPath,
                     const QString &replyEventId,
                     const QString &editEventId,
                     const QVariantMap &usernames,
                     CustomEmojiModel *cem);

    /// \brief Send edit instructions (.e.g s/hallo/hello/)
    ///
    /// This will automatically edit the last message posted and send the sed
    /// instruction to IRC.
    void postEdit(const QString &text);

private:
    Quotient::Connection *m_connection = nullptr;
    NeoChatRoom *m_room = nullptr;
};
