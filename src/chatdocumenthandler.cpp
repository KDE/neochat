// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chatdocumenthandler.h"

#include <QQmlFile>
#include <QQmlFileSelector>
#include <QQuickTextDocument>
#include <QStringBuilder>
#include <QTextBlock>
#include <QTextDocument>

#include "neochatroom.h"

ChatDocumentHandler::ChatDocumentHandler(QObject *parent)
    : QObject(parent)
    , m_document(nullptr)
    , m_cursorPosition(-1)
    , m_selectionStart(-1)
    , m_selectionEnd(-1)
{
}

QQuickTextDocument *ChatDocumentHandler::document() const
{
    return m_document;
}

void ChatDocumentHandler::setDocument(QQuickTextDocument *document)
{
    if (document == m_document) {
        return;
    }

    if (m_document) {
        m_document->textDocument()->disconnect(this);
    }
    m_document = document;
    Q_EMIT documentChanged();
}

int ChatDocumentHandler::cursorPosition() const
{
    return m_cursorPosition;
}

void ChatDocumentHandler::setCursorPosition(int position)
{
    if (position == m_cursorPosition) {
        return;
    }

    m_cursorPosition = position;
    Q_EMIT cursorPositionChanged();
}

int ChatDocumentHandler::selectionStart() const
{
    return m_selectionStart;
}

void ChatDocumentHandler::setSelectionStart(int position)
{
    if (position == m_selectionStart) {
        return;
    }

    m_selectionStart = position;
    Q_EMIT selectionStartChanged();
}

int ChatDocumentHandler::selectionEnd() const
{
    return m_selectionEnd;
}

void ChatDocumentHandler::setSelectionEnd(int position)
{
    if (position == m_selectionEnd) {
        return;
    }

    m_selectionEnd = position;
    Q_EMIT selectionEndChanged();
}

QTextCursor ChatDocumentHandler::textCursor() const
{
    QTextDocument *doc = textDocument();
    if (!doc) {
        return QTextCursor();
    }

    QTextCursor cursor = QTextCursor(doc);
    if (m_selectionStart != m_selectionEnd) {
        cursor.setPosition(m_selectionStart);
        cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(m_cursorPosition);
    }
    return cursor;
}

QTextDocument *ChatDocumentHandler::textDocument() const
{
    if (!m_document) {
        return nullptr;
    }

    return m_document->textDocument();
}

NeoChatRoom *ChatDocumentHandler::room() const
{
    return m_room;
}

void ChatDocumentHandler::setRoom(NeoChatRoom *room)
{
    if (m_room == room) {
        return;
    }

    m_room = room;
    Q_EMIT roomChanged();
}

QVariantMap ChatDocumentHandler::getAutocompletionInfo(bool isAutocompleting)
{
    QTextCursor cursor = textCursor();

    if (cursor.block().text() == m_lastState) {
        // ignore change, it was caused by autocompletion
        return QVariantMap{
            {"type", AutoCompletionType::Ignore},
        };
    }

    QString text = cursor.block().text();
    QString textBeforeCursor = text;
    textBeforeCursor.truncate(m_cursorPosition);

    QString autoCompletePrefix = textBeforeCursor.section(" ", -1);

    if (autoCompletePrefix.isEmpty()) {
        return QVariantMap{
            {"type", AutoCompletionType::None},
        };
    }

    if (autoCompletePrefix.startsWith("@") || autoCompletePrefix.startsWith(":") || autoCompletePrefix.startsWith("/")) {
        m_autoCompleteBeginPosition = textBeforeCursor.lastIndexOf(" ") + 1; // 1 == space

        if (autoCompletePrefix.startsWith("@")) {
            autoCompletePrefix.remove(0, 1);
            return QVariantMap{
                {"keyword", autoCompletePrefix},
                {"type", AutoCompletionType::User},
            };
        }

        if (autoCompletePrefix.startsWith("/")) {
            return QVariantMap{
                {"keyword", autoCompletePrefix},
                {"type", AutoCompletionType::Command},
            };
        }

        if (!isAutocompleting) {
            return QVariantMap{
                {"keyword", autoCompletePrefix},
                {"type", AutoCompletionType::Emoji},
            };
        } else {
            return QVariantMap{
                {"type", AutoCompletionType::Ignore},
                {"keyword", autoCompletePrefix},
            };
        }
    }

    return QVariantMap{
        {"type", AutoCompletionType::None},
    };
}

void ChatDocumentHandler::replaceAutoComplete(const QString &word)
{
    QTextCursor cursor = textCursor();
    if (cursor.block().text() == m_lastState) {
        m_document->textDocument()->undo();
    }
    cursor.beginEditBlock();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.removeSelectedText();
    cursor.deletePreviousChar();
    while (!cursor.atBlockStart()) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

        if (cursor.selectedText() == " ") {
            cursor.movePosition(QTextCursor::NextCharacter);
            break;
        }
    }

    cursor.insertHtml(word);

    // Add space after autocomplete if not already there
    if (!cursor.block().text().endsWith(QStringLiteral(" "))) {
        cursor.insertText(QStringLiteral(" "));
    }

    m_lastState = cursor.block().text();
    cursor.endEditBlock();
}
