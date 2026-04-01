// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "chattextitemhelper.h"
#include "enums/blocktype.h"

namespace Blocks
{
class Block : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The block type.
     */
    Q_PROPERTY(Blocks::Type type MEMBER type CONSTANT)

    /**
     * @brief The display string for the block.
     */
    Q_PROPERTY(QString display MEMBER display CONSTANT)

    /**
     * @brief The attributes for the block.
     */
    Q_PROPERTY(QVariantMap attributes MEMBER attributes CONSTANT)

public:
    Block(QObject *parent = nullptr);
    Block(Type type, const QString &display, const QVariantMap &attributes, QObject *parent = nullptr);

    Type type = Other;
    QString display;
    QVariantMap attributes;

    virtual QVariant toVariant() const;

    bool operator==(const Block &right) const
    {
        return type == right.type && display == right.display && attributes == right.attributes;
    }

    bool isEmpty() const
    {
        return type == Other;
    }
};

using BlockPtr = std::unique_ptr<Blocks::Block>;
using BlockPtrs = std::vector<BlockPtr>;
using BlockPtrsIt = BlockPtrs::iterator;

class TextBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The block type.
     */
    Q_PROPERTY(ChatTextItemHelper *item READ item CONSTANT)

public:
    TextBlock(QObject *parent = nullptr);
    TextBlock(Type type, const QTextDocumentFragment &text, const QVariantMap &attributes, QObject *parent = nullptr);

    ChatTextItemHelper *item() const;

    QVariant toVariant() const override;

private:
    ChatTextItemHelper *m_item;
};
}
