// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <events/roommessageevent.h>

#include "neochatroom.h"

class CustomEmojiModel;
class NeoChatRoom;

/// \brief Handles user interactions with NeoChat (joining room, creating room,
/// sending message). Account management is handled by Controller.
class ActionsHandler : public QObject
{
    Q_OBJECT

    /// \brief The room that messages will be sent to.
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    explicit ActionsHandler(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

Q_SIGNALS:
    void roomChanged();
    void showEffect(QString effect);

public Q_SLOTS:

    /// \brief Post a message.
    ///
    /// This also interprets commands if any.
    void handleMessage();

private:
    NeoChatRoom *m_room = nullptr;
    void checkEffects();
};

QString markdownToHTML(const QString &markdown);
