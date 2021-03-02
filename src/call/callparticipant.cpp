// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "callparticipant.h"

NeoChatUser *CallParticipant::user() const
{
    return m_user;
}

bool CallParticipant::hasCamera() const
{
    return m_hasCamera;
}

CallParticipant::CallParticipant(QObject *parent)
    : QObject(parent)
{
}

void CallParticipant::initCamera(QQuickItem *item)
{
    QTimer::singleShot(500, this, [=] {
        Q_EMIT initialized(item);
    });
}
