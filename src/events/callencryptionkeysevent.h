// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <Quotient/events/roomevent.h>

namespace Quotient
{

class CallEncryptionKeysEvent : public RoomEvent
{
public:
    QUO_EVENT(CallEncryptionKeysEvent, "io.element.call.encryption_keys");
    explicit CallEncryptionKeysEvent(const QJsonObject &obj)
        : RoomEvent(obj)
    {
    }
};
}
