// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatkeyhelper.h"

#include "chatbarcache.h"
#include "chattextitemhelper.h"
#include "clipboard.h"
#include "neochatroom.h"

ChatKeyHelper::ChatKeyHelper(QObject *parent)
    : QObject(parent)
{
}

bool ChatKeyHelper::handleKey(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    switch (key) {
    case Qt::Key_V:
        return vKey(modifiers);
    case Qt::Key_Up:
        return up(modifiers);
    case Qt::Key_Down:
        return down();
    case Qt::Key_Tab:
        return tab();
    case Qt::Key_Delete:
        return deleteChar();
    case Qt::Key_Backspace:
        return backspace();
    case Qt::Key_Enter:
    case Qt::Key_Return:
        return insertReturn();
    default:
        return false;
    }
}

bool ChatKeyHelper::vKey(Qt::KeyboardModifiers modifiers)
{
    if (!textItem) {
        return false;
    }

    if (modifiers.testFlag(Qt::ControlModifier)) {
        return pasteImage();
    }
    return false;
}

bool ChatKeyHelper::up(Qt::KeyboardModifiers modifiers)
{
    if (!textItem) {
        return false;
    }

    if (modifiers.testFlag(Qt::ControlModifier)) {
        room->replyLastMessage();
        return true;
    }

    if (textItem->isCompleting) {
        Q_EMIT unhandledUp(true);
        return true;
    }

    QTextCursor cursor = textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    if (cursor.blockNumber() == 0 && cursor.block().layout()->lineForTextPosition(cursor.positionInBlock()).lineNumber() == 0) {
        Q_EMIT unhandledUp(false);
        return true;
    }
    return false;
}

bool ChatKeyHelper::down()
{
    if (!textItem) {
        return false;
    }
    if (textItem->isCompleting) {
        Q_EMIT unhandledDown(true);
        return true;
    }

    QTextCursor cursor = textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    if (cursor.blockNumber() == cursor.document()->blockCount() - 1
        && cursor.block().layout()->lineForTextPosition(cursor.positionInBlock()).lineNumber() == (cursor.block().layout()->lineCount() - 1)) {
        Q_EMIT unhandledDown(false);
        return true;
    }
    return false;
}

bool ChatKeyHelper::tab()
{
    if (!textItem) {
        return false;
    }
    if (textItem->isCompleting) {
        Q_EMIT unhandledTab(true);
        return true;
    }

    QTextCursor cursor = textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    if (cursor.currentList() && textItem->canIndentListMoreAtCursor()) {
        textItem->indentListMoreAtCursor();
        return true;
    }
    return false;
}

bool ChatKeyHelper::deleteChar()
{
    if (!textItem) {
        return false;
    }

    QTextCursor cursor = textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    if (cursor.position() >= textItem->document()->characterCount() - textItem->fixedEndChars().length() - 1) {
        Q_EMIT unhandledDelete();
        return true;
    }
    return false;
}

bool ChatKeyHelper::backspace()
{
    if (!textItem) {
        return false;
    }

    QTextCursor cursor = textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    if (cursor.position() <= textItem->fixedStartChars().length()) {
        if (cursor.currentList() && textItem->canIndentListLessAtCursor()) {
            textItem->indentListLessAtCursor();
            return true;
        }
        Q_EMIT unhandledBackspace();
        return true;
    }
    return false;
}

bool ChatKeyHelper::insertReturn()
{
    if (!textItem) {
        return false;
    }
    if (textItem->isCompleting) {
        Q_EMIT unhandledReturn(true);
        return true;
    }

    QTextCursor cursor = textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    cursor.insertBlock();
    return true;
}

bool ChatKeyHelper::pasteImage()
{
    if (!textItem) {
        return false;
    }
    const auto savePath = Clipboard().saveImage();
    if (!savePath.isEmpty()) {
        Q_EMIT imagePasted(savePath);
    }
    return false;
}

#include "moc_chatkeyhelper.cpp"
