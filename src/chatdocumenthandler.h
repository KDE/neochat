// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QFont>
#include <QObject>
#include <QQuickTextDocument>
#include <QTextCursor>
#include <QUrl>

class QTextDocument;
class NeoChatRoom;
class Controller;

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
        Command,
        None,
        Ignore,
    };
    Q_ENUM(AutoCompletionType)

    explicit ChatDocumentHandler(QObject *parent = nullptr);

    [[nodiscard]] QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    [[nodiscard]] int cursorPosition() const;
    void setCursorPosition(int position);

    [[nodiscard]] int selectionStart() const;
    void setSelectionStart(int position);

    [[nodiscard]] int selectionEnd() const;
    void setSelectionEnd(int position);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /// This function will look at the current QTextCursor and determine if there
    /// is the possibility to autocomplete it.
    Q_INVOKABLE QVariantMap getAutocompletionInfo(bool isAutocompleting);
    Q_INVOKABLE void replaceAutoComplete(const QString &word);

Q_SIGNALS:
    void documentChanged();
    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();
    void roomChanged();
    void joinRoom(QString roomName);

private:
    [[nodiscard]] QTextCursor textCursor() const;
    [[nodiscard]] QTextDocument *textDocument() const;

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
