// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <Quotient/events/roomevent.h>

namespace Quotient
{

class CallNotifyEvent : public RoomEvent
{
public:
    QUO_EVENT(CallNotifyEvent, "org.matrix.msc4075.call.notify");
    explicit CallNotifyEvent(const QJsonObject &obj);
};
