// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQuickTextDocument>
#include <QTextCursor>

#include "completionmodel.h"
#include "userlistmodel.h"

class QTextDocument;
class NeoChatRoom;
class SyntaxHighlighter;

class ChatDocumentHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    Q_PROPERTY(CompletionModel *completionModel READ completionModel NOTIFY completionModelChanged)

    Q_PROPERTY(NeoChatRoom *room READ room NOTIFY roomChanged)

public:
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

    Q_INVOKABLE void complete(int index);

    void updateCompletions();
    CompletionModel *completionModel() const;
Q_SIGNALS:
    void documentChanged();
    void cursorPositionChanged();
    void roomChanged();
    void completionModelChanged();
    void selectionStartChanged();
    void selectionEndChanged();

private:
    int completionStartIndex() const;

    QQuickTextDocument *m_document;

    NeoChatRoom *m_room = nullptr;
    bool completionVisible = false;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    SyntaxHighlighter *m_highlighter = nullptr;

    CompletionModel::AutoCompletionType m_completionType = CompletionModel::None;

    CompletionModel *m_completionModel = nullptr;
};
