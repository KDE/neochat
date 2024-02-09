// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "sharehandler.h"

ShareHandler::ShareHandler(QObject *parent)
    : QObject(parent)
{}

QString ShareHandler::text() const
{
    return m_text;
}

void ShareHandler::setText(const QString &text)
{
    if (text == m_text) {
        return;
    }
    m_text = text;
    Q_EMIT textChanged();
}

QString ShareHandler::room() const
{
    return m_room;
}

void ShareHandler::setRoom(const QString &roomId)
{
    if (roomId == m_room) {
        return;
    }
    m_room = roomId;
    Q_EMIT roomChanged();
}

