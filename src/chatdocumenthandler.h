// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQuickTextDocument>
#include <QTextCursor>

#include "models/completionmodel.h"
#include "models/userlistmodel.h"
#include "neochatroom.h"

class QTextDocument;
class NeoChatRoom;
class SyntaxHighlighter;

class ChatDocumentHandler : public QObject
{
    Q_OBJECT

    /**
     * @brief Is the instance being used to handle an edit message.
     *
     * This is needed to ensure that the text and mentions are saved and retrieved
     * from the correct parameters in the assigned room.
     */
    Q_PROPERTY(bool isEdit READ isEdit WRITE setIsEdit NOTIFY isEditChanged)
    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    Q_PROPERTY(CompletionModel *completionModel READ completionModel NOTIFY completionModelChanged)

    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    explicit ChatDocumentHandler(QObject *parent = nullptr);

    [[nodiscard]] bool isEdit() const;
    void setIsEdit(bool edit);

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
    void isEditChanged();
    void documentChanged();
    void cursorPositionChanged();
    void roomChanged();
    void completionModelChanged();
    void selectionStartChanged();
    void selectionEndChanged();

private:
    int completionStartIndex() const;

    bool m_isEdit;

    QQuickTextDocument *m_document;

    NeoChatRoom *m_room = nullptr;
    bool completionVisible = false;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    QString getText() const;
    void pushMention(const Mention mention) const;

    SyntaxHighlighter *m_highlighter = nullptr;

    CompletionModel::AutoCompletionType m_completionType = CompletionModel::None;

    CompletionModel *m_completionModel = nullptr;
};
