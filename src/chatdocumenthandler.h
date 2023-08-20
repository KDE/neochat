// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQuickTextDocument>
#include <QTextCursor>

#include "models/completionmodel.h"
#include "neochatroom.h"

class QTextDocument;
class NeoChatRoom;
class SyntaxHighlighter;

/**
 * @class ChatDocumentHandler
 *
 * Handle the QQuickTextDocument of a qml text item.
 *
 * The class provides functionality to highlight text in the text document as well
 * as providing completion functionality via a CompletionModel.
 *
 * The ChatDocumentHandler is also linked to a NeoChatRoom to provide functionality
 * to save the chat document text when switching between rooms.
 *
 * To get the full functionality the cursor position and text selection information
 * need to be passed in. For example:
 *
 * @code{.qml}
 * import QtQuick 2.0
 * import QtQuick.Controls 2.15 as QQC2
 *
 * import org.kde.kirigami 2.12 as Kirigami
 * import org.kde.neochat 1.0
 *
 * QQC2.TextArea {
 *      id: textField
 *
 *      // Set this to a NeoChatRoom object.
 *      property var room
 *
 *      ChatDocumentHandler {
 *          id: documentHandler
 *          document: textField.textDocument
 *          cursorPosition: textField.cursorPosition
 *          selectionStart: textField.selectionStart
 *          selectionEnd: textField.selectionEnd
 *          mentionColor: Kirigami.Theme.linkColor
 *          errorColor: Kirigami.Theme.negativeTextColor
 *          room: textField.room
 *      }
 * }
 * @endcode
 *
 * @sa QQuickTextDocument, CompletionModel, NeoChatRoom
 */
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

    /**
     * @brief The QQuickTextDocument that is being handled.
     */
    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)

    /**
     * @brief The current saved cursor position.
     */
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)

    /**
     * @brief The start position of any currently selected text.
     */
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)

    /**
     * @brief The end position of any currently selected text.
     */
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    /**
     * @brief The current CompletionModel.
     *
     * This is typically provided to a qml component to visualise the current
     * completion results.
     */
    Q_PROPERTY(CompletionModel *completionModel READ completionModel NOTIFY completionModelChanged)

    /**
     * @brief The current room that the the text document is being handled for.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief The color to highlight user mentions.
     */
    Q_PROPERTY(QColor mentionColor READ mentionColor WRITE setMentionColor NOTIFY mentionColorChanged)

    /**
     * @brief The color to highlight spelling errors.
     */
    Q_PROPERTY(QColor errorColor READ errorColor WRITE setErrorColor NOTIFY errorColorChanged)

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

    [[nodiscard]] QColor mentionColor() const;
    void setMentionColor(const QColor &color);

    [[nodiscard]] QColor errorColor() const;
    void setErrorColor(const QColor &color);

Q_SIGNALS:
    void isEditChanged();
    void documentChanged();
    void cursorPositionChanged();
    void roomChanged();
    void completionModelChanged();
    void selectionStartChanged();
    void selectionEndChanged();
    void errorColorChanged();
    void mentionColorChanged();

private:
    int completionStartIndex() const;

    bool m_isEdit = false;

    QPointer<QQuickTextDocument> m_document;

    QPointer<NeoChatRoom> m_room;
    bool completionVisible = false;

    QColor m_mentionColor;
    QColor m_errorColor;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    QString getText() const;
    void pushMention(const Mention mention) const;

    SyntaxHighlighter *m_highlighter = nullptr;

    CompletionModel::AutoCompletionType m_completionType = CompletionModel::None;

    CompletionModel *m_completionModel = nullptr;
};
