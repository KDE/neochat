// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "enums/blocktype.h"

namespace Blocks
{
struct Block {
    Type type = Other;
    QString display;
    QVariantMap attributes;

    bool operator==(const Block &right) const
    {
        return type == right.type && display == right.display && attributes == right.attributes;
    }

    bool isEmpty() const
    {
        return type == Other;
    }
};
}
