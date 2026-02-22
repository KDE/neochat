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

bool ChatKeyHelper::up(Qt::KeyboardModifiers modifiers)
{
    if (!m_textItem) {
        return false;
    }

    if (modifiers.testFlag(Qt::ControlModifier)) {
        const auto room = m_textItem->room();
        if (!room) {
            return false;
        }
        room->replyLastMessage();
        return true;
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
    return selectLeft(cursor, modifiers);
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
    return selectRight(cursor, modifiers);
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

bool ChatKeyHelper::selectLeft(QTextCursor &cursor, Qt::KeyboardModifiers modifiers)
{
    if ((cursor.hasSelection() || !cursor.charFormat().isAnchor()) && !modifiers.testFlag(Qt::ShiftModifier)) {
        return false;
    }

    // We need to rearrange the selection from right to left.
    const auto selectionStart = cursor.selectionStart();
    cursor.setPosition(cursor.selectionEnd());
    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, cursor.position() - selectionStart);

    if (!cursor.charFormat().isAnchor()) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        m_textItem->setSelection(cursor.selectionEnd(), cursor.selectionStart());
        return true;
    }

    const auto hRef = cursor.charFormat().anchorHref();
    auto currentCharFormat = cursor.charFormat();
    while (currentCharFormat.isAnchor() && currentCharFormat.anchorHref() == hRef && cursor.position() > 0) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        currentCharFormat = cursor.charFormat();
    }

    m_textItem->setSelection(cursor.selectionEnd(), cursor.selectionStart());
    return true;
}

bool ChatKeyHelper::selectRight(QTextCursor &cursor, Qt::KeyboardModifiers modifiers)
{
    if ((cursor.hasSelection() && !modifiers.testFlag(Qt::ShiftModifier)) || cursor.atBlockEnd()) {
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

    m_textItem->setSelection(cursor.selectionStart(), cursor.selectionEnd());
    return true;
}

bool ChatKeyHelper::nextWordLeft(QTextCursor &cursor, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers.testFlag(Qt::ControlModifier)) {
        return false;
    }

    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    // Cross any whitespace.
    while (cursor.selectedText() == u' ') {
        cursor.setPosition(cursor.selectionStart());
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    }
    if (!cursor.charFormat().isAnchor()) {
        return false;
    }
    // Cross link.
    while (cursor.charFormat().isAnchor()) {
        cursor.setPosition(cursor.selectionStart());
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    }

    m_textItem->setCursorPosition(cursor.selectionStart());
    return true;
}

bool ChatKeyHelper::nextWordRight(QTextCursor &cursor, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers.testFlag(Qt::ControlModifier)) {
        return false;
    }

    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    // Cross any whitespace.
    while (cursor.selectedText() == u' ') {
        cursor.setPosition(cursor.selectionEnd());
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    if (!cursor.charFormat().isAnchor()) {
        return false;
    }
    // Cross link.
    while (cursor.charFormat().isAnchor()) {
        cursor.setPosition(cursor.selectionEnd());
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    m_textItem->setCursorPosition(cursor.selectionEnd());
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
    bool selectingLink = false;
    cursor.beginEditBlock();
    cursor.setPosition(m_textItem->selectionStart());
    if (cursor.charFormat().isAnchor()) {
        selectingLink = true;
    }
    if (!selectingLink) {
        cursor.movePosition(QTextCursor::NextCharacter);
        if (cursor.charFormat().isAnchor()) {
            selectingLink = true;
        }
    }
    if (!selectingLink) {
        cursor.setPosition(m_textItem->selectionEnd());
        if (cursor.charFormat().isAnchor()) {
            selectingLink = true;
        }
    }
    if (!selectingLink) {
        return;
    }
    // Wind all the way to the left of the link.
    cursor.setPosition(m_textItem->selectionStart());
    const auto hRef = cursor.charFormat().anchorHref();
    auto currentCharFormat = cursor.charFormat();
    while (currentCharFormat.isAnchor() && currentCharFormat.anchorHref() == hRef && cursor.position() > 0) {
        cursor.movePosition(QTextCursor::PreviousCharacter);
        currentCharFormat = cursor.charFormat();
    }
    cursor.endEditBlock();
    selectRight(cursor);
}

void ChatKeyHelper::checkLinkFormat(int position, int charsRemoved, int charsAdded)
{
    if (!m_textItem || charsRemoved > charsAdded || charsAdded - charsRemoved != 1) {
        return;
    }
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }

    bool nextToLink = false;
    cursor.setPosition(position);
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
    cursor.setPosition(position);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    cursor.setCharFormat({});
    cursor.endEditBlock();
}

#include "moc_chatkeyhelper.cpp"
