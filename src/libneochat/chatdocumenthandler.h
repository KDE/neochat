// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QTextCursor>

#include "chatbarcache.h"
#include "enums/chatbartype.h"
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
    QML_ELEMENT

    /**
     * @brief The QQuickTextDocument that is being handled.
     */
    Q_PROPERTY(ChatBarType::Type type READ type WRITE setType NOTIFY typeChanged)

    /**
     * @brief The QML text Item the ChatDocumentHandler is handling.
     */
    Q_PROPERTY(QQuickItem *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    /**
     * @brief The current CompletionModel.
     *
     * This is typically provided to a qml component to visualise the current
     * completion results.
     */
    Q_PROPERTY(CompletionModel *completionModel READ completionModel CONSTANT)

    /**
     * @brief The current room that the text document is being handled for.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    explicit ChatDocumentHandler(QObject *parent = nullptr);

    ChatBarType::Type type() const;
    void setType(ChatBarType::Type type);

    QQuickItem *textItem() const;
    void setTextItem(QQuickItem *textItem);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    ChatBarCache *chatBarCache() const;

    Q_INVOKABLE void complete(int index);

    CompletionModel *completionModel() const;

    /**
     * @brief Update the mentions in @p document when editing a message.
     */
    Q_INVOKABLE void updateMentions(const QString &editId);

Q_SIGNALS:
    void typeChanged();
    void textItemChanged();
    void roomChanged();

public Q_SLOTS:
    void updateCompletion() const;

private:
    ChatBarType::Type m_type = ChatBarType::None;
    QPointer<QQuickItem> m_textItem;
    QTextDocument *document() const;

    int completionStartIndex() const;

    QPointer<NeoChatRoom> m_room;

    int cursorPosition() const;

    QString getText() const;
    void pushMention(const Mention mention) const;

    SyntaxHighlighter *m_highlighter = nullptr;

    CompletionModel *m_completionModel = nullptr;
};
