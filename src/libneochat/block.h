// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "enums/blocktype.h"

namespace Blocks
{
class Block
{
    Q_GADGET
    QML_VALUE_TYPE(block)

    /**
     * @brief The block type.
     */
    Q_PROPERTY(Blocks::Type type MEMBER type)

    /**
     * @brief The display string for the block.
     */
    Q_PROPERTY(QString display MEMBER display)

    /**
     * @brief The attributes for the block.
     */
    Q_PROPERTY(QVariantMap attributes MEMBER attributes)

public:
    Block() = default;
    Block(Type type, const QString &display, const QVariantMap &attributes);

    Type type = Other;
    QString display;
    QVariantMap attributes;

    QVariant toVariant() const;

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
