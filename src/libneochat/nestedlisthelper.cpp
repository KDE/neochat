/**
 * Nested list helper
 *
 * SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "nestedlisthelper_p.h"

#include <QKeyEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextList>

NestedListHelper::NestedListHelper()
{
    listBottomMargin = 12;
    listTopMargin = 12;
    listNoMargin = 0;
}

bool NestedListHelper::handleBeforeKeyPressEvent(QKeyEvent *event, const QTextCursor &cursor)
{
    // Only attempt to handle Backspace while on a list
    if ((event->key() != Qt::Key_Backspace) || (!cursor.currentList())) {
        return false;
    }

    bool handled = false;

    if (!cursor.hasSelection() && cursor.currentList() && event->key() == Qt::Key_Backspace && cursor.atBlockStart()) {
        handleOnIndentLess(cursor);
        handled = true;
    }

    return handled;
}

bool NestedListHelper::canIndent(const QTextCursor &textCursor) const
{
    const auto block = textCursor.block();
    if (textCursor.isNull() || !block.isValid()) {
        return false;
    }

    if (!block.textList()) {
        return true;
    }

    return block.textList()->format().indent() < 3;
}

bool NestedListHelper::canDedent(const QTextCursor &textCursor) const
{
    const auto block = textCursor.block();
    if (textCursor.isNull() || !block.isValid()) {
        return false;
    }

    if (!block.textList()) {
        return false;
    }

    return block.textList()->format().indent() > 0;
}

bool NestedListHelper::handleAfterKeyPressEvent(QKeyEvent *event, const QTextCursor &cursor)
{
    // Only attempt to handle Backspace and Return
    if ((event->key() != Qt::Key_Backspace) && (event->key() != Qt::Key_Return)) {
        return false;
    }

    bool handled = false;

    if (!cursor.hasSelection() && cursor.currentList()) {
        // Check if we're on the last list item.
        // itemNumber is zero indexed
        QTextBlock currentBlock = cursor.block();
        if (cursor.currentList()->count() == cursor.currentList()->itemNumber(currentBlock) + 1) {
            // Last block in this list, but may have just gained another list below.
            if (currentBlock.next().textList()) {
                reformatList(cursor.block());
            }
            reformatList(cursor.block());

            // No need to reformatList in this case. reformatList is slow.
            if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Backspace)) {
                handled = true;
            }
        } else {
            reformatList(cursor.block());
        }
    }
    return handled;
}

void NestedListHelper::processList(QTextList *list)
{
    QTextBlock block = list->item(0);
    const int thisListIndent = list->format().indent();

    QTextCursor cursor = QTextCursor(block);
    list = cursor.createList(list->format());
    bool processingSubList = false;
    while (block.next().textList() != nullptr) {
        block = block.next();

        QTextList *nextList = block.textList();
        const int nextItemIndent = nextList->format().indent();
        if (nextItemIndent < thisListIndent) {
            return;
        } else if (nextItemIndent > thisListIndent) {
            if (processingSubList) {
                continue;
            }
            processingSubList = true;
            processList(nextList);
        } else {
            processingSubList = false;
            list->add(block);
        }
    }
    //     delete nextList;
    //     nextList = 0;
}

void NestedListHelper::reformatList(QTextBlock block)
{
    if (block.textList()) {
        const int minimumIndent = block.textList()->format().indent();

        // Start at the top of the list
        while (block.previous().textList() != nullptr) {
            if (block.previous().textList()->format().indent() < minimumIndent) {
                break;
            }
            block = block.previous();
        }

        processList(block.textList());
    }
}

QTextCursor NestedListHelper::topOfSelection(QTextCursor cursor)
{
    if (cursor.hasSelection()) {
        cursor.setPosition(qMin(cursor.position(), cursor.anchor()));
    }
    return cursor;
}

QTextCursor NestedListHelper::bottomOfSelection(QTextCursor cursor)
{
    if (cursor.hasSelection()) {
        cursor.setPosition(qMax(cursor.position(), cursor.anchor()));
    }
    return cursor;
}

void NestedListHelper::handleOnIndentMore(const QTextCursor &textCursor)
{
    QTextCursor cursor = textCursor;

    QTextListFormat listFmt;
    if (!cursor.currentList()) {
        QTextListFormat::Style style;
        cursor = topOfSelection(textCursor);
        cursor.movePosition(QTextCursor::PreviousBlock);
        if (cursor.currentList()) {
            style = cursor.currentList()->format().style();
        } else {
            cursor = bottomOfSelection(textCursor);
            cursor.movePosition(QTextCursor::NextBlock);

            if (cursor.currentList()) {
                style = cursor.currentList()->format().style();
            } else {
                style = QTextListFormat::ListDisc;
            }
        }
        handleOnBulletType(style, textCursor);
    } else {
        listFmt = cursor.currentList()->format();
        listFmt.setIndent(listFmt.indent() + 1);

        cursor.createList(listFmt);
        reformatList(textCursor.block());
    }
}

void NestedListHelper::handleOnIndentLess(const QTextCursor &textCursor)
{
    QTextCursor cursor = textCursor;
    QTextList *currentList = cursor.currentList();
    if (!currentList) {
        return;
    }
    QTextListFormat listFmt = currentList->format();
    if (listFmt.indent() > 1) {
        listFmt.setIndent(listFmt.indent() - 1);
        cursor.createList(listFmt);
        reformatList(cursor.block());
    } else {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.setBlockFormat(bfmt);
        reformatList(cursor.block().next());
    }
}

void NestedListHelper::handleOnBulletType(QTextListFormat::Style style, QTextCursor cursor)
{
    if (cursor.isNull()) {
        return;
    }
    QTextListFormat::Style currentListStyle = QTextListFormat::ListStyleUndefined;
    if (cursor.currentList()) {
        currentListStyle = cursor.currentList()->format().style();
    }
    if (style != currentListStyle && style != QTextListFormat::ListStyleUndefined) {
        QTextList *currentList = cursor.currentList();
        QTextListFormat listFmt;

        cursor.beginEditBlock();

        if (currentList) {
            listFmt = currentList->format();
            listFmt.setStyle(style);
            currentList->setFormat(listFmt);
        } else {
            listFmt.setStyle(style);
            cursor.createList(listFmt);
        }

        cursor.endEditBlock();
    } else {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.setBlockFormat(bfmt);
    }

    reformatList(cursor.block());
}
