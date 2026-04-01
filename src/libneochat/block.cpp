// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "block.h"

using namespace Blocks;

Block::Block(Type type, const QString &display, const QVariantMap &attributes)
    : type(type)
    , display(display)
    , attributes(attributes)
{
}

QVariant Block::toVariant() const
{
    return QVariant::fromValue(*this);
}

#include "moc_block.cpp"
