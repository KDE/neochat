// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatmarkdownhelper.h"

#include <QTextDocument>
#include <qtextcursor.h>

#include "chatdocumenthandler.h"

struct MarkdownSyntax {
    QLatin1String sequence;
    bool closable = false;
    bool lineStart = false;
    RichFormat::Format format;
};

static const QList<MarkdownSyntax> syntax = {
    MarkdownSyntax{.sequence = "*"_L1, .closable = true, .format = RichFormat::Italic},
    MarkdownSyntax{.sequence = "**"_L1, .closable = true, .format = RichFormat::Bold},
    MarkdownSyntax{.sequence = "# "_L1, .lineStart = true, .format = RichFormat::Heading1},
    MarkdownSyntax{.sequence = "## "_L1, .lineStart = true, .format = RichFormat::Heading2},
    MarkdownSyntax{.sequence = "### "_L1, .lineStart = true, .format = RichFormat::Heading3},
    MarkdownSyntax{.sequence = "#### "_L1, .lineStart = true, .format = RichFormat::Heading4},
    MarkdownSyntax{.sequence = "##### "_L1, .lineStart = true, .format = RichFormat::Heading5},
    MarkdownSyntax{.sequence = "###### "_L1, .lineStart = true, .format = RichFormat::Heading6},
    MarkdownSyntax{.sequence = "> "_L1, .lineStart = true, .format = RichFormat::Quote},
    MarkdownSyntax{.sequence = "* "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = "- "_L1, .lineStart = true, .format = RichFormat::UnorderedList},
    MarkdownSyntax{.sequence = "1. "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = "1) "_L1, .lineStart = true, .format = RichFormat::OrderedList},
    MarkdownSyntax{.sequence = "`"_L1, .closable = true, .format = RichFormat::InlineCode},
    MarkdownSyntax{.sequence = "```"_L1, .lineStart = true, .format = RichFormat::Code},
    MarkdownSyntax{.sequence = "~~"_L1, .closable = true, .format = RichFormat::Strikethrough},
    MarkdownSyntax{.sequence = "__"_L1, .closable = true, .format = RichFormat::Underline},
};

static std::optional<bool> checkSequence(const QString &currentString, const QString &nextChar, bool lineStart = false)
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

    if (partialMatches.length() > 0) {
        return false;
    }
    if (fullMatch) {
        return true;
    }
    return std::nullopt;
}

static std::optional<MarkdownSyntax> syntaxForSequence(const QString &sequence)
{
    const auto it = std::find_if(syntax.cbegin(), syntax.cend(), [sequence](const MarkdownSyntax &syntax) {
        return syntax.sequence == sequence;
    });
    if (it == syntax.cend()) {
        return std::nullopt;
    }
    return *it;
}

ChatMarkdownHelper::ChatMarkdownHelper(ChatDocumentHandler *parent)
    : QObject(parent)
{
    Q_ASSERT(parent);

    connectDocument();
    connect(parent, &ChatDocumentHandler::textItemChanged, this, &ChatMarkdownHelper::connectDocument);
}

QTextDocument *ChatMarkdownHelper::document() const
{
    const auto documentHandler = qobject_cast<ChatDocumentHandler *>(parent());
    if (!documentHandler) {
        return nullptr;
    }

    if (!documentHandler->textItem()) {
        return nullptr;
    }

    const auto quickDocument = qvariant_cast<QQuickTextDocument *>(documentHandler->textItem()->property("textDocument"));
    return quickDocument ? quickDocument->textDocument() : nullptr;
}

void ChatMarkdownHelper::connectDocument()
{
    disconnect();

    if (document()) {
        m_startPos = qobject_cast<ChatDocumentHandler *>(parent())->textItem()->property("cursorPosition").toInt();
        m_endPos = m_startPos;
        if (m_startPos == 0) {
            m_currentState = Pre;
        }

        connect(document(), &QTextDocument::contentsChange, this, &ChatMarkdownHelper::checkMarkdown);
    }
}

void ChatMarkdownHelper::checkMarkdown(int position, int charsRemoved, int charsAdded)
{
    qWarning() << "1" << m_currentState << m_startPos << m_endPos;

    if (!document()) {
        return;
    }

    auto cursor = QTextCursor(document());
    if (cursor.isNull()) {
        return;
    }

    if (charsRemoved - charsAdded > 0) {
        if (position < m_startPos) {
            m_startPos = position;
        }
        m_endPos -= charsRemoved;
        cursor.setPosition(m_endPos);
        cursor.setPosition(m_endPos + (cursor.atBlockEnd() ? 0 : 1), QTextCursor::KeepAnchor);
        const auto nextChar = cursor.selectedText();
        m_currentState = m_startPos == 0 || nextChar == u' ' ? Pre : None;
        qWarning() << "2" << m_currentState << m_startPos << m_endPos;
        return;
    }

    for (auto i = 1; i <= charsAdded - charsRemoved; ++i) {
        cursor.setPosition(m_startPos);
        cursor.setPosition(m_endPos, QTextCursor::KeepAnchor);
        const auto currentMarkdown = cursor.selectedText();
        cursor.setPosition(m_endPos);
        cursor.setPosition(m_endPos + 1, QTextCursor::KeepAnchor);
        const auto nextChar = cursor.selectedText();
        cursor.setPosition(m_startPos);

        const auto result = checkSequence(currentMarkdown, nextChar, cursor.atBlockStart());
        qWarning() << result;

        switch (m_currentState) {
        case None:
            if (nextChar == u' ' || cursor.atBlockEnd()) {
                m_currentState = Pre;
            }
            ++m_startPos;
            m_endPos = m_startPos;
            break;
        case Pre:
            if (!result && RichFormat::formatsAtCursor(cursor).length() == 0) {
                m_currentState = None;
            } else if (result && !*result) {
                m_currentState = Started;
                ++m_endPos;
                break;
            }
            ++m_startPos;
            m_endPos = m_startPos;
            break;
        case Started:
            if (!result) {
                m_currentState = Pre;
                ++m_startPos;
                m_endPos = m_startPos;
                break;
            } else if (!*result) {
                ++m_endPos;
                break;
            }
            complete();
            break;
        }
    }

    qWarning() << "2" << m_currentState << m_startPos << m_endPos;
}

void ChatMarkdownHelper::complete()
{
    auto cursor = QTextCursor(document());
    if (cursor.isNull()) {
        return;
    }
    cursor.setPosition(m_startPos);
    cursor.setPosition(m_endPos, QTextCursor::KeepAnchor);
    const auto syntax = syntaxForSequence(cursor.selectedText());
    cursor.removeSelectedText();

    if (m_currentFormats.contains(syntax->format)) {
        m_currentFormats.remove(syntax->format);
    } else if (syntax->closable) {
        m_currentFormats.insert(syntax->format, m_startPos);
    }

    ++m_startPos;

    const auto documentHandler = qobject_cast<ChatDocumentHandler *>(parent());
    if (syntax) {
        documentHandler->textItem()->setProperty("cursorPosition", m_startPos);
        documentHandler->setFormat(syntax->format);
    }

    m_currentState = Pre;
    m_endPos = m_startPos;

    documentHandler->textItem()->setProperty("cursorPosition", m_startPos);
}

void ChatMarkdownHelper::handleExternalFormatChange()
{
    auto cursor = QTextCursor(document());
    if (cursor.isNull()) {
        return;
    }
    cursor.setPosition(m_startPos);
    m_currentState = RichFormat::formatsAtCursor(cursor).length() > 0 ? Pre : None;
    qWarning() << "3" << m_currentState << m_startPos << m_endPos;
}

#include "moc_chatmarkdownhelper.cpp"
