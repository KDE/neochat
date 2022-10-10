// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>

#include <QTextCursor>

#include "userlistmodel.h"

class QTextDocument;
class QQuickTextDocument;
class NeoChatRoom;
class SyntaxHighlighter;
class CompletionModel;

class ChatDocumentHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(CompletionModel *completionModel READ completionModel NOTIFY completionModelChanged)

    Q_PROPERTY(NeoChatRoom *room READ room NOTIFY roomChanged)

public:
    enum AutoCompletionType {
        User,
        Room,
        Emoji,
        Command,
        None,
    };
    Q_ENUM(AutoCompletionType)

    explicit ChatDocumentHandler(QObject *parent = nullptr);

    [[nodiscard]] QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    [[nodiscard]] int cursorPosition() const;
    void setCursorPosition(int position);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    Q_INVOKABLE void complete(int index);

    void updateCompletions();
    CompletionModel *completionModel() const;
Q_SIGNALS:
    void documentChanged();
    void cursorPositionChanged();
    void roomChanged();
    void completionModelChanged();

private:
    int completionStartIndex() const;

    QQuickTextDocument *m_document;

    NeoChatRoom *m_room = nullptr;
    bool completionVisible = false;

    int m_cursorPosition;

    SyntaxHighlighter *m_highlighter = nullptr;

    AutoCompletionType m_completionType = None;

    CompletionModel *m_completionModel = nullptr;
};

Q_DECLARE_METATYPE(ChatDocumentHandler::AutoCompletionType);
