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

QVariantMap ChatDocumentHandler::getAutocompletionInfo()
{
    QTextCursor cursor = textCursor();

    if (cursor.block().text() == m_lastState) {
        // ignore change, it was caused by autocompletion
        return QVariantMap{
            {"type", AutoCompletionType::Ignore},
        };
    }
    if (m_cursorPosition != m_autoCompleteBeginPosition && m_cursorPosition != m_autoCompleteEndPosition) {
        // we moved our cursor, so cancel autocompletion
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

    if (autoCompletePrefix.startsWith("@") || autoCompletePrefix.startsWith(":")) {
        m_autoCompleteBeginPosition = textBeforeCursor.lastIndexOf(" ") + 1; // 1 == space

        if (autoCompletePrefix.startsWith("@")) {
            autoCompletePrefix.remove(0, 1);
            return QVariantMap{
                {"keyword", autoCompletePrefix},
                {"type", AutoCompletionType::User},
            };
        }
        return QVariantMap{
            {"keyword", autoCompletePrefix},
            {"type", AutoCompletionType::Emoji},
        };
    }

    return QVariantMap{
        {"type", AutoCompletionType::None},
    };
}

void ChatDocumentHandler::postMessage(const QString &text, const QString &attachementPath, const QString &replyEventId, const QString &editEventId) const
{
    if (!m_room || !m_document) {
        return;
    }

    QString cleanedText = text;

    cleanedText = cleanedText.trimmed();

    if (attachementPath.length() > 0) {
        m_room->uploadFile(attachementPath, cleanedText);
    }

    if (cleanedText.length() == 0) {
        return;
    }

    auto messageEventType = RoomMessageEvent::MsgType::Text;

    const QString rainbowPrefix = QStringLiteral("/rainbow ");
    const QString mePrefix = QStringLiteral("/me ");
    const QString noticePrefix = QStringLiteral("/notice ");

    if (cleanedText.indexOf(rainbowPrefix) == 0) {
        cleanedText = cleanedText.remove(0, rainbowPrefix.length());
        QString rainbowText;
        QStringList rainbowColors{"#ff2b00", "#ff5500", "#ff8000", "#ffaa00", "#ffd500", "#ffff00", "#d4ff00", "#aaff00", "#80ff00", "#55ff00", "#2bff00", "#00ff00", "#00ff2b", "#00ff55", "#00ff80", "#00ffaa", "#00ffd5", "#00ffff",
                                  "#00d4ff", "#00aaff", "#007fff", "#0055ff", "#002bff", "#0000ff", "#2a00ff", "#5500ff", "#7f00ff", "#aa00ff", "#d400ff", "#ff00ff", "#ff00d4", "#ff00aa", "#ff0080", "#ff0055", "#ff002b", "#ff0000"};

        for (int i = 0; i < cleanedText.length(); i++) {
            rainbowText = rainbowText % QStringLiteral("<font color='") % rainbowColors.at(i % rainbowColors.length()) % "'>" % cleanedText.at(i) % "</font>";
        }
        m_room->postHtmlMessage(cleanedText, rainbowText, messageEventType, replyEventId, editEventId);
        return;
    }

    if (cleanedText.indexOf(mePrefix) == 0) {
        cleanedText = cleanedText.remove(0, mePrefix.length());
        messageEventType = RoomMessageEvent::MsgType::Emote;
    } else if (cleanedText.indexOf(noticePrefix) == 0) {
        cleanedText = cleanedText.remove(0, noticePrefix.length());
        messageEventType = RoomMessageEvent::MsgType::Notice;
    }
    m_room->postMessage(cleanedText, messageEventType, replyEventId, editEventId);
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
    m_lastState = cursor.block().text();
    cursor.endEditBlock();
}
