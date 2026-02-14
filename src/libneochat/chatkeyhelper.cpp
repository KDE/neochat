// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatkeyhelper.h"

#include "chattextitemhelper.h"
#include "clipboard.h"
#include "neochatroom.h"
#include "richformat.h"

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
        return insertReturn(modifiers);
    case Qt::Key_Escape:
    case Qt::Key_Cancel:
        return cancel();
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
        const auto room = textItem->room();
        if (!room) {
            return false;
        }
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
    if (cursor.isNull() || cursor.hasSelection()) {
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

bool ChatKeyHelper::insertReturn(Qt::KeyboardModifiers modifiers)
{
    if (!textItem) {
        return false;
    }

    bool shiftPressed = modifiers.testFlag(Qt::ShiftModifier);
    if (shiftPressed && !sendMessageWithEnter) {
        Q_EMIT unhandledReturn(false);
        return true;
    }

    if (!shiftPressed && textItem->isCompleting) {
        Q_EMIT unhandledReturn(true);
        return true;
    }

    if (!shiftPressed && sendMessageWithEnter) {
        Q_EMIT unhandledReturn(false);
        return true;
    }

    QTextCursor cursor = textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }

    // If there was a heading we always want to revert to Paragraph format.
    auto newBlockFormat = RichFormat::blockFormatForFormat(RichFormat::Paragraph);
    auto newCharFormat = cursor.charFormat();
    newCharFormat.merge(RichFormat::charFormatForFormat(static_cast<RichFormat::Format>(cursor.blockFormat().headingLevel()), true));
    cursor.insertBlock(newBlockFormat, newCharFormat);
    return true;
}

bool ChatKeyHelper::cancel()
{
    if (!textItem) {
        return false;
    }
    if (textItem->isCompleting) {
        Q_EMIT closeCompletion();
        return true;
    }
    return false;
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
