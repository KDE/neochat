// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatkeyhelper.h"

#include "chattextitemhelper.h"
#include "clipboard.h"
#include "neochatroom.h"
#include "richformat.h"
#include <qtextcursor.h>

ChatKeyHelper::ChatKeyHelper(QObject *parent)
    : QObject(parent)
{
}

ChatTextItemHelper *ChatKeyHelper::textItem() const
{
    return m_textItem;
}

void ChatKeyHelper::setTextItem(ChatTextItemHelper *textItem)
{
    if (textItem == m_textItem) {
        return;
    }

    if (m_textItem) {
        m_textItem->disconnect(this);
    }
    m_textItem = textItem;
    if (m_textItem) {
        connect(m_textItem, &ChatTextItemHelper::contentsChange, this, &ChatKeyHelper::checkLinkFormat);
        connect(m_textItem, &ChatTextItemHelper::selectedTextChanged, this, &ChatKeyHelper::checkMouseSelection);
    }
}

bool ChatKeyHelper::handleKey(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    switch (key) {
    case Qt::Key_V:
        return vKey(modifiers);
    case Qt::Key_X:
        return xKey(modifiers);
    case Qt::Key_Up:
        return up(modifiers);
    case Qt::Key_Down:
        return down();
    case Qt::Key_Left:
        return left(modifiers);
    case Qt::Key_Right:
        return right(modifiers);
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
    if (modifiers.testFlag(Qt::ControlModifier)) {
        return pasteImage();
    }
    return false;
}

bool ChatKeyHelper::xKey(Qt::KeyboardModifiers modifiers)
{
    if (!m_textItem || !modifiers.testFlag(Qt::ControlModifier)) {
        return false;
    }
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    if (cursor.selectionStart() == 0 && cursor.selectionEnd() == cursor.document()->characterCount() - 1) {
        m_textItem->cut();
        if (cursor.currentList()) {
            QTextBlockFormat blockFormat;
            blockFormat.setObjectIndex(-1);
            cursor.setBlockFormat(blockFormat);
        }
        return true;
    }
    return false;
}

bool ChatKeyHelper::up(Qt::KeyboardModifiers modifiers)
{
    if (!m_textItem) {
        return false;
    }

    if (modifiers.testFlag(Qt::ControlModifier)) {
        if (!room) {
            return false;
        }
        const auto lastId = room->lastMessageId();
        if (!lastId.isEmpty()) {
            Q_EMIT requestReply(lastId);
            return true;
        }
    }

    if (m_textItem->isCompleting) {
        Q_EMIT unhandledUp(true);
        return true;
    }

    QTextCursor cursor = m_textItem->textCursor();
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
    if (!m_textItem) {
        return false;
    }
    if (m_textItem->isCompleting) {
        Q_EMIT unhandledDown(true);
        return true;
    }

    QTextCursor cursor = m_textItem->textCursor();
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

bool ChatKeyHelper::left(Qt::KeyboardModifiers modifiers)
{
    if (!m_textItem) {
        return false;
    }
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    bool ctrlLeft = nextWordLeft(cursor, modifiers);
    if (ctrlLeft) {
        return true;
    }
    return selectLeft(cursor);
}

bool ChatKeyHelper::right(Qt::KeyboardModifiers modifiers)
{
    if (!m_textItem) {
        return false;
    }
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    bool ctrlRight = nextWordRight(cursor, modifiers);
    if (ctrlRight) {
        return true;
    }
    return selectRight(cursor);
}

bool ChatKeyHelper::tab()
{
    if (!m_textItem) {
        return false;
    }
    if (m_textItem->isCompleting) {
        Q_EMIT unhandledTab(true);
        return true;
    }

    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    if (cursor.currentList() && m_textItem->canIndentListMoreAtCursor()) {
        m_textItem->indentListMoreAtCursor();
        return true;
    }
    return false;
}

bool ChatKeyHelper::deleteChar()
{
    if (!m_textItem) {
        return false;
    }

    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull() || cursor.hasSelection()) {
        return false;
    }
    if (cursor.position() >= m_textItem->document()->characterCount() - m_textItem->fixedEndChars().length() - 1) {
        Q_EMIT unhandledDelete();
        return true;
    }
    return selectRight(cursor);
}

bool ChatKeyHelper::backspace()
{
    if (!m_textItem) {
        return false;
    }

    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    if (cursor.blockFormat().headingLevel() > 0 && m_textItem->plainText().length() <= m_textItem->fixedStartChars().length() + 1) {
        m_textItem->mergeFormatOnCursor(RichFormat::Paragraph);
        return false;
    }
    if (cursor.position() <= m_textItem->fixedStartChars().length()) {
        if (cursor.currentList() && m_textItem->canIndentListLessAtCursor()) {
            m_textItem->indentListLessAtCursor();
            return true;
        }
        Q_EMIT unhandledBackspace();
        return true;
    }
    return selectLeft(cursor);
}

bool ChatKeyHelper::insertReturn(Qt::KeyboardModifiers modifiers)
{
    if (!m_textItem) {
        return false;
    }

    bool shiftPressed = modifiers.testFlag(Qt::ShiftModifier);
    if (shiftPressed && !sendMessageWithEnter) {
        Q_EMIT unhandledReturn(false);
        return true;
    }

    if (!shiftPressed && m_textItem->isCompleting) {
        Q_EMIT unhandledReturn(true);
        return true;
    }

    if (!shiftPressed && sendMessageWithEnter) {
        Q_EMIT unhandledReturn(false);
        return true;
    }

    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }

    // If we are not a heading or are in a list just pass enter as normal.
    if (cursor.blockFormat().headingLevel() <= 0 || cursor.currentList()) {
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
    if (!m_textItem) {
        return false;
    }
    if (m_textItem->isCompleting) {
        Q_EMIT closeCompletion();
        return true;
    }
    return false;
}

bool ChatKeyHelper::pasteImage()
{
    if (!m_textItem) {
        return false;
    }
    const auto savePath = Clipboard().saveImage();
    if (!savePath.isEmpty()) {
        Q_EMIT imagePasted(savePath);
    }
    return false;
}

inline int linkLeftPos(QTextCursor cursor)
{
    // For when we have a selection otherwise cursor movement is weird.
    cursor.setPosition(cursor.selectionStart());
    if (cursor.charFormat().isAnchor()) {
        const auto hRef = cursor.charFormat().anchorHref();
        auto currentCharFormat = cursor.charFormat();
        while (currentCharFormat.isAnchor() && currentCharFormat.anchorHref() == hRef && cursor.position() > 0) {
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
            currentCharFormat = cursor.charFormat();
        }
        return cursor.position();
    }
    cursor.movePosition(QTextCursor::NextCharacter);
    if (cursor.charFormat().isAnchor()) {
        return cursor.position() - 1;
    }
    return -1;
}

inline int linkRightPost(QTextCursor cursor)
{
    if (!cursor.charFormat().isAnchor()) {
        return -1;
    }
    const auto hRef = cursor.charFormat().anchorHref();
    auto currentCharFormat = cursor.charFormat();
    while (currentCharFormat.isAnchor() && currentCharFormat.anchorHref() == hRef && cursor.position() < cursor.block().length() - 1) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        currentCharFormat = cursor.charFormat();
    }
    return cursor.position() - 1;
}

bool ChatKeyHelper::selectLeft(QTextCursor &cursor)
{
    bool selecting = cursor.selectionStart() == cursor.position();
    if (!cursor.charFormat().isAnchor() || (selecting && cursor.hasSelection() && !cursor.charFormat().isAnchor())) {
        return false;
    }

    if (!cursor.charFormat().isAnchor()) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        const auto start = selecting ? cursor.selectionEnd() : cursor.selectionStart();
        const auto end = selecting ? cursor.selectionStart() : cursor.selectionEnd();
        m_textItem->setSelection(start, end);
        return true;
    }

    const auto hRef = cursor.charFormat().anchorHref();
    auto currentCharFormat = cursor.charFormat();
    while (currentCharFormat.isAnchor() && currentCharFormat.anchorHref() == hRef && cursor.position() > 0) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        currentCharFormat = cursor.charFormat();
    }

    const auto start = selecting ? cursor.selectionEnd() : cursor.selectionStart();
    const auto end = selecting ? cursor.selectionStart() : cursor.selectionEnd();
    m_textItem->setSelection(start, end);
    return true;
}

bool ChatKeyHelper::selectRight(QTextCursor &cursor)
{
    bool selecting = cursor.selectionEnd() == cursor.position();
    if ((selecting && cursor.hasSelection() && cursor.charFormat().isAnchor()) || cursor.atBlockEnd()) {
        return false;
    }

    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    if (!cursor.charFormat().isAnchor()) {
        return false;
    }

    const auto hRef = cursor.charFormat().anchorHref();
    auto currentCharFormat = cursor.charFormat();
    while (currentCharFormat.isAnchor() && currentCharFormat.anchorHref() == hRef && cursor.position() < cursor.block().length() - 1) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        currentCharFormat = cursor.charFormat();
    }
    if (!currentCharFormat.isAnchor()) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    }

    const auto start = selecting ? cursor.selectionStart() : cursor.selectionEnd();
    const auto end = selecting ? cursor.selectionEnd() : cursor.selectionStart();
    m_textItem->setSelection(start, end);
    return true;
}

bool ChatKeyHelper::nextWordLeft(QTextCursor &cursor, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers.testFlag(Qt::ControlModifier)) {
        return false;
    }

    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    // Cross any whitespace.
    while (cursor.selectedText() == u' ' && !cursor.atBlockStart()) {
        cursor.setPosition(cursor.selectionStart());
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    }
    if (!cursor.charFormat().isAnchor()) {
        cursor.setPosition(cursor.selectionEnd());
        return false;
    }
    // Cross link.
    while (cursor.charFormat().isAnchor() && !cursor.atBlockStart()) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    }

    if (modifiers.testFlag(Qt::ShiftModifier)) {
        bool selecting = cursor.selectionStart() == cursor.position();
        const auto start = selecting ? cursor.selectionEnd() : cursor.selectionStart();
        const auto end = selecting ? cursor.selectionStart() : cursor.selectionEnd();
        m_textItem->setSelection(start, end);
    } else {
        m_textItem->setCursorPosition(cursor.selectionStart());
    }
    return true;
}

bool ChatKeyHelper::nextWordRight(QTextCursor &cursor, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers.testFlag(Qt::ControlModifier)) {
        return false;
    }

    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    if (!cursor.charFormat().isAnchor()) {
        return false;
    }
    // Cross link.
    while (cursor.charFormat().isAnchor() && !cursor.atBlockEnd()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    // Cross any whitespace.
    while (cursor.selectedText().endsWith(u' ') && !cursor.atBlockEnd()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        bool selecting = cursor.selectionEnd() == cursor.position();
        const auto start = selecting ? cursor.selectionStart() : cursor.selectionEnd();
        const auto end = selecting ? cursor.selectionEnd() - 1 : cursor.selectionStart();
        m_textItem->setSelection(start, end);
    } else {
        m_textItem->setCursorPosition(cursor.selectionEnd() - 1);
    }
    return true;
}

void ChatKeyHelper::checkMouseSelection()
{
    if (!m_textItem) {
        return;
    }
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }
    if (!cursor.hasSelection()) {
        return;
    }
    int leftPos = linkLeftPos(cursor);
    int rightPos = linkRightPost(cursor);
    if ((cursor.selectionStart() == leftPos && cursor.selectionEnd() == rightPos) || rightPos == cursor.selectionStart() || leftPos == -1 || rightPos == -1) {
        return;
    }

    bool cursorleft = cursor.position() == cursor.selectionStart();
    if (cursorleft) {
        m_textItem->setSelection(rightPos == -1 ? cursor.selectionEnd() : rightPos, leftPos == -1 ? cursor.selectionStart() : leftPos);
    }
    m_textItem->setSelection(leftPos == -1 ? cursor.selectionStart() : leftPos, rightPos == -1 ? cursor.selectionEnd() : rightPos);
}

void ChatKeyHelper::checkLinkFormat(int position, int lengthBefore, int lengthAfter)
{
    if (!m_textItem || lengthBefore > lengthAfter || lengthAfter - lengthBefore != 1) {
        return;
    }
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }

    bool nextToLink = false;
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, position + lengthAfter);
    if (cursor.charFormat().isAnchor()) {
        nextToLink = true;
    }
    // Note 2 because a cursor on the left of a link will not show it in the format.
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, 2);
    if (cursor.charFormat().isAnchor()) {
        nextToLink = true;
    }
    if (!nextToLink) {
        return;
    }

    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, position + lengthAfter);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    cursor.setCharFormat({});
    cursor.endEditBlock();
}

#include "moc_chatkeyhelper.cpp"
