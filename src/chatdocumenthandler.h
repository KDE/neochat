/**
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QFont>
#include <QObject>
#include <QTextCursor>
#include <QUrl>

class QTextDocument;
class QQuickTextDocument;
class NeoChatRoom;

class ChatDocumentHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    enum AutoCompletionType {
        User,
        Emoji,
        None,
        Ignore,
    };
    Q_ENUM(AutoCompletionType)

    explicit ChatDocumentHandler(QObject *object = nullptr);

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    int cursorPosition() const;
    void setCursorPosition(int position);

    int selectionStart() const;
    void setSelectionStart(int position);

    int selectionEnd() const;
    void setSelectionEnd(int position);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    Q_INVOKABLE void postMessage(const QString &attachmentPath, const QString &replyEventId) const;

    /// This function will look at the current QTextCursor and determine if there
    /// is the posibility to autocomplete it.
    Q_INVOKABLE QVariantMap getAutocompletionInfo();
    Q_INVOKABLE void replaceAutoComplete(const QString &word);

Q_SIGNALS:
    void documentChanged();
    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();
    void roomChanged();

private:
    QTextCursor textCursor() const;
    QTextDocument *textDocument() const;

    QQuickTextDocument *m_document;

    NeoChatRoom *m_room;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    int m_autoCompleteBeginPosition = -1;
    int m_autoCompleteEndPosition = -1;

    QString m_lastState;
};

Q_DECLARE_METATYPE(ChatDocumentHandler::AutoCompletionType);
