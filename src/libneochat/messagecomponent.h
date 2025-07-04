// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "enums/messagecomponenttype.h"

struct MessageComponent {
    MessageComponentType::Type type = MessageComponentType::Other;
    QString content;
    QVariantMap attributes;

    bool operator==(const MessageComponent &right) const
    {
        return type == right.type && content == right.content && attributes == right.attributes;
    }

    bool isEmpty() const
    {
        return type == MessageComponentType::Other;
    }
};
