// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatkeyhelper.h"

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
        connect(m_textItem, &ChatTextItemHelper::textItemChanged, this, &ChatKeyHelper::textItemChanged);
    }

    Q_EMIT textItemChanged();
}

void ChatKeyHelper::up()
{
    if (!m_textItem) {
        return;
    }
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }
    if (cursor.blockNumber() == 0 && cursor.block().layout()->lineForTextPosition(cursor.positionInBlock()).lineNumber() == 0) {
        Q_EMIT unhandledUp();
        return;
    }
    cursor.movePosition(QTextCursor::Up);
    m_textItem->setCursorPosition(cursor.position());
}

void ChatKeyHelper::down()
{
    if (!m_textItem) {
        return;
    }
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }
    if (cursor.blockNumber() == cursor.document()->blockCount() - 1
        && cursor.block().layout()->lineForTextPosition(cursor.positionInBlock()).lineNumber() == (cursor.block().layout()->lineCount() - 1)) {
        Q_EMIT unhandledDown();
        return;
    }
    cursor.movePosition(QTextCursor::Down);
    m_textItem->setCursorPosition(cursor.position());
}

void ChatKeyHelper::tab()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }
    if (cursor.currentList() && m_textItem->canIndentListMoreAtCursor()) {
        m_textItem->indentListMoreAtCursor();
        return;
    }
    cursor.insertText(u"	"_s);
}

void ChatKeyHelper::deleteChar()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }
    if (cursor.position() >= m_textItem->document()->characterCount() - m_textItem->fixedEndChars().length() - 1) {
        Q_EMIT unhandledDelete();
        return;
    }
    cursor.deleteChar();
}

void ChatKeyHelper::backspace()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }
    if (cursor.position() <= m_textItem->fixedStartChars().length()) {
        if (cursor.currentList() && m_textItem->canIndentListLessAtCursor()) {
            m_textItem->indentListLessAtCursor();
            return;
        }
        Q_EMIT unhandledBackspace();
        return;
    }
    cursor.deletePreviousChar();
}

void ChatKeyHelper::insertReturn()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }
    cursor.insertBlock();
}

#include "moc_chatkeyhelper.cpp"
