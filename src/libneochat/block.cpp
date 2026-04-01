// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "block.h"

using namespace Blocks;

Block::Block(QObject *parent)
    : QObject(parent)
{
}

Block::Block(Type type, const QString &display, const QVariantMap &attributes, QObject *parent)
    : QObject(parent)
    , type(type)
    , display(display)
    , attributes(attributes)
{
}

QVariant Block::toVariant() const
{
    return QVariant::fromValue(this);
}

TextBlock::TextBlock(QObject *parent)
    : Block(parent)
{
}

TextBlock::TextBlock(Type type, const QTextDocumentFragment &text, const QVariantMap &attributes, QObject *parent)
    : Block(type, {}, attributes, parent)
    , m_item(new ChatTextItemHelper(this))
{
    m_item->setInitialFragment(text);
    if (type == Blocks::Quote) {
        m_item->setFixedChars(u"“"_s, u"”"_s);
    }
}

ChatTextItemHelper *TextBlock::item() const
{
    return m_item;
}

QVariant TextBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

#include "moc_block.cpp"
