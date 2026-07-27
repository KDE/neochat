// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatmarkdownhelper.h"

#include <QTextCursor>
#include <QTextDocument>
#include <qtextcursor.h>

#include "chattextitemhelper.h"
#include "richformat.h"

namespace
{
struct MarkdownSyntax {
    QLatin1String sequence;
    bool closable = false;
    bool lineStart = false;
    RichFormat::Format format;
};

const QList<MarkdownSyntax> syntax = {
    MarkdownSyntax{.sequence = "*"_L1, .closable = true, .format = RichFormat::Italic},
    MarkdownSyntax{.sequence = "**"_L1, .closable = true, .format = RichFormat::Bold},
    MarkdownSyntax{.sequence = "# "_L1, .lineStart = true, .format = RichFormat::Heading1},
    MarkdownSyntax{.sequence = " # "_L1, .lineStart = true, .format = RichFormat::Heading1},
    MarkdownSyntax{.sequence = "  # "_L1, .lineStart = true, .format = RichFormat::Heading1},
    MarkdownSyntax{.sequence = "   # "_L1, .lineStart = true, .format = RichFormat::Heading1},
    MarkdownSyntax{.sequence = "## "_L1, .lineStart = true, .format = RichFormat::Heading2},
    MarkdownSyntax{.sequence = " ## "_L1, .lineStart = true, .format = RichFormat::Heading2},
    MarkdownSyntax{.sequence = "  ## "_L1, .lineStart = true, .format = RichFormat::Heading2},
    MarkdownSyntax{.sequence = "   ## "_L1, .lineStart = true, .format = RichFormat::Heading2},
    MarkdownSyntax{.sequence = "### "_L1, .lineStart = true, .format = RichFormat::Heading3},
    MarkdownSyntax{.sequence = " ### "_L1, .lineStart = true, .format = RichFormat::Heading3},
    MarkdownSyntax{.sequence = "  ### "_L1, .lineStart = true, .format = RichFormat::Heading3},
    MarkdownSyntax{.sequence = "   ### "_L1, .lineStart = true, .format = RichFormat::Heading3},
    MarkdownSyntax{.sequence = "#### "_L1, .lineStart = true, .format = RichFormat::Heading4},
    MarkdownSyntax{.sequence = " #### "_L1, .lineStart = true, .format = RichFormat::Heading4},
    MarkdownSyntax{.sequence = "  #### "_L1, .lineStart = true, .format = RichFormat::Heading4},
    MarkdownSyntax{.sequence = "   #### "_L1, .lineStart = true, .format = RichFormat::Heading4},
    MarkdownSyntax{.sequence = "##### "_L1, .lineStart = true, .format = RichFormat::Heading5},
    MarkdownSyntax{.sequence = " ##### "_L1, .lineStart = true, .format = RichFormat::Heading5},
    MarkdownSyntax{.sequence = "  ##### "_L1, .lineStart = true, .format = RichFormat::Heading5},
    MarkdownSyntax{.sequence = "   ##### "_L1, .lineStart = true, .format = RichFormat::Heading5},
    MarkdownSyntax{.sequence = "###### "_L1, .lineStart = true, .format = RichFormat::Heading6},
    MarkdownSyntax{.sequence = " ###### "_L1, .lineStart = true, .format = RichFormat::Heading6},
    MarkdownSyntax{.sequence = "  ###### "_L1, .lineStart = true, .format = RichFormat::Heading6},
    MarkdownSyntax{.sequence = "   ###### "_L1, .lineStart = true, .format = RichFormat::Heading6},
    MarkdownSyntax{.sequence = ">"_L1, .lineStart = true, .format = RichFormat::Quote},
    MarkdownSyntax{.sequence = " >"_L1, .lineStart = true, .format = RichFormat::Quote},
    MarkdownSyntax{.sequence = "  >"_L1, .lineStart = true, .format = RichFormat::Quote},
    MarkdownSyntax{.sequence = "   >"_L1, .lineStart = true, .format = RichFormat::Quote},
    MarkdownSyntax{.sequence = "> "_L1, .lineStart = true, .format = RichFormat::Quote},
    MarkdownSyntax{.sequence = " > "_L1, .lineStart = true, .format = RichFormat::Quote},
    MarkdownSyntax{.sequence = "  > "_L1, .lineStart = true, .format = RichFormat::Quote},
    MarkdownSyntax{.sequence = "   > "_L1, .lineStart = true, .format = RichFormat::Quote},
    MarkdownSyntax{.sequence = "* "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = " * "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = "  * "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = "   * "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = "- "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = " - "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = "  - "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = "   - "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = "1. "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = " 1. "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = "  1. "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = "   1. "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = "1) "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = " 1) "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = "  1) "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = "   1) "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = "`"_L1, .closable = true, .format = RichFormat::InlineCode},
    MarkdownSyntax{.sequence = "```"_L1, .lineStart = true, .format = RichFormat::Code},
    MarkdownSyntax{.sequence = " ```"_L1, .lineStart = true, .format = RichFormat::Code},
    MarkdownSyntax{.sequence = "  ```"_L1, .lineStart = true, .format = RichFormat::Code},
    MarkdownSyntax{.sequence = "   ```"_L1, .lineStart = true, .format = RichFormat::Code},
    MarkdownSyntax{.sequence = "~~"_L1, .closable = true, .format = RichFormat::Strikethrough},
    MarkdownSyntax{.sequence = "_"_L1, .closable = true, .format = RichFormat::Underline},
};

std::pair<std::optional<MarkdownSyntax>, QList<MarkdownSyntax>> checkSequence(const QString &currentString, const QString &nextChar, bool lineStart = false)
{
    QList<MarkdownSyntax> partialMatches;
    std::optional<MarkdownSyntax> fullMatch = std::nullopt;
    auto it = syntax.cbegin();
    while ((it = std::find_if(it,
                              syntax.cend(),
                              [currentString, nextChar](const MarkdownSyntax &syntax) {
                                  return syntax.sequence == currentString || syntax.sequence.startsWith(QString(currentString + nextChar));
                              }))
           != syntax.cend()) {
        if (it->lineStart ? lineStart : true) {
            if (it->sequence == currentString) {
                fullMatch = *it;
            } else {
                partialMatches += *it;
            }
        }
        ++it;
    }

    return {fullMatch, partialMatches};
}

bool checkSequenceBackwards(const QString &currentString)
{
    auto it = syntax.cbegin();
    while ((it = std::find_if(it,
                              syntax.cend(),
                              [currentString](const MarkdownSyntax &syntax) {
                                  return syntax.sequence.endsWith(currentString);
                              }))
           != syntax.cend()) {
        return true;
    }
    return false;
}

std::optional<MarkdownSyntax> syntaxForSequence(const QString &sequence)
{
    const auto it = std::find_if(syntax.cbegin(), syntax.cend(), [sequence](const MarkdownSyntax &syntax) {
        return syntax.sequence == sequence;
    });
    if (it == syntax.cend()) {
        return std::nullopt;
    }
    return *it;
}
}

bool ChatMarkdownHelper::richTextActive = true;

ChatMarkdownHelper::ChatMarkdownHelper(QObject *parent)
    : QObject(parent)
{
}

ChatTextItemHelper *ChatMarkdownHelper::textItem() const
{
    return m_textItem;
}

void ChatMarkdownHelper::setTextItem(ChatTextItemHelper *textItem)
{
    if (textItem == m_textItem) {
        return;
    }

    if (m_textItem) {
        m_textItem->disconnect(this);
    }

    m_textItem = textItem;

    if (m_textItem) {
        connect(m_textItem, &ChatTextItemHelper::textItemChanged, this, &ChatMarkdownHelper::updateStart);
        connect(m_textItem, &ChatTextItemHelper::cursorPositionChanged, this, [this](bool fromContentsChange) {
            if (!fromContentsChange) {
                updateStart();
            }
        });
        connect(m_textItem, &ChatTextItemHelper::contentsChange, this, &ChatMarkdownHelper::checkMarkdown);
    }

    Q_EMIT textItemChanged();
}

void ChatMarkdownHelper::updateStart()
{
    if (!m_textItem) {
        return;
    }
    const auto newCursorPosition = m_textItem->cursorPosition();
    if (newCursorPosition) {
        updatePosition(*newCursorPosition);
    }
}

void ChatMarkdownHelper::checkMarkdown(int position, int charsRemoved, int charsAdded)
{
    if (!richTextActive) {
        return;
    }

    updatePosition(position);

    // This can happen when formatting is applied.
    if (charsAdded == charsRemoved) {
        return;
    } else if ((m_textItem->textFormat() && m_textItem->textFormat() == Qt::TextFormat::PlainText) || m_textItem->isCompleting || charsRemoved > charsAdded || charsAdded - charsRemoved > 1) {
        updatePosition(std::max(0, position - charsRemoved + charsAdded));
        return;
    }

    checkMarkdownForward();
}

void ChatMarkdownHelper::updatePosition(int position)
{
    if (position == m_endPos) {
        return;
    }

    m_startPos = position;
    m_endPos = position;

    if (m_startPos <= 0) {
        return;
    }

    auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }
    cursor.setPosition(m_startPos);
    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    while (checkSequenceBackwards(cursor.selectedText()) && m_startPos > 0) {
        --m_startPos;
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    }
}

void ChatMarkdownHelper::checkMarkdownForward()
{
    auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }

    cursor.setPosition(m_startPos);
    const auto atBlockStart = cursor.atBlockStart();
    cursor.setPosition(m_endPos, QTextCursor::KeepAnchor);
    const auto currentMarkdown = cursor.selectedText();
    cursor.setPosition(m_endPos);
    cursor.setPosition(m_endPos + 1, QTextCursor::KeepAnchor);
    const auto nextChar = cursor.selectedText();

    const auto [fullMatch, partialMatches] = checkSequence(currentMarkdown, nextChar, atBlockStart);
    if (!fullMatch && partialMatches.isEmpty()) {
        ++m_startPos;
        m_endPos = m_startPos;
        return;
    }

    if (!partialMatches.isEmpty()) {
        const auto syntax = partialMatches.constFirst();
        const QString completeCursor = currentMarkdown + nextChar;
        // Check if we can close syntax like '`' (one char) or '~~' (two char). If so, add a space too so it's obvious that block is done and format also
        // doesn't merge.
        if (syntax.closable && RichFormat::hasFormat(cursor, syntax.format) && completeCursor == syntax.sequence) {
            ++m_endPos;
            cursor.clearSelection();
            cursor.insertText(u" "_s);
            checkMarkdownForward();
            m_endPos = m_startPos;
            return;
        }

        ++m_endPos;
        return;
    }

    complete();
}

void ChatMarkdownHelper::complete()
{
    auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }
    cursor.beginEditBlock();
    cursor.setPosition(m_startPos);
    cursor.setPosition(m_endPos, QTextCursor::KeepAnchor);
    const auto syntax = syntaxForSequence(cursor.selectedText());
    if (!syntax) {
        return;
    }
    cursor.removeSelectedText();

    cursor.setPosition(m_startPos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    const auto nextChar = cursor.selectedText();
    const auto [fullMatch, partialMatches] = checkSequence({}, nextChar, cursor.atBlockStart());

    cursor.setPosition(m_startPos);
    cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);

    const auto formatType = RichFormat::typeForFormat(syntax->format);
    if (formatType == RichFormat::Block) {
        // We just hit enter and don't want an extra empty line.
        if (cursor.atBlockStart()) {
            cursor.deletePreviousChar();
        }
        Q_EMIT unhandledBlockFormat(syntax->format);
    } else {
        m_textItem->mergeFormatOnCursor(syntax->format, cursor);
    }

    m_startPos = fullMatch || !partialMatches.isEmpty() ? m_startPos : m_startPos + 1;
    m_endPos = fullMatch || !partialMatches.isEmpty() ? m_startPos + 1 : m_startPos;

    cursor.endEditBlock();
}

#include "moc_chatmarkdownhelper.cpp"
