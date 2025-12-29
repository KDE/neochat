// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QTextCursor>
#include <qnamespace.h>
#include <qtextdocumentfragment.h>

#include "chatbarcache.h"
#include "chatmarkdownhelper.h"
#include "enums/chatbartype.h"
#include "enums/richformat.h"
#include "neochatroom.h"
#include "nestedlisthelper_p.h"

class QTextDocument;

class QmlTextItemWrapper;
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
     * @brief The current room that the text document is being handled for.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief The QML text Item the ChatDocumentHandler is handling.
     */
    Q_PROPERTY(QQuickItem *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    /**
     * @brief Whether the cursor is currently on the first line.
     */
    Q_PROPERTY(bool atFirstLine READ atFirstLine NOTIFY atFirstLineChanged)

    /**
     * @brief Whether the cursor is cuurently on the last line.
     */
    Q_PROPERTY(bool atLastLine READ atLastLine NOTIFY atLastLineChanged)

public:
    enum InsertPosition {
        Cursor,
        Start,
        End,
    };

    explicit ChatDocumentHandler(QObject *parent = nullptr);

    ChatBarType::Type type() const;
    void setType(ChatBarType::Type type);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    QQuickItem *textItem() const;
    void setTextItem(QQuickItem *textItem);

    ChatDocumentHandler *previousDocumentHandler() const;
    void setPreviousDocumentHandler(ChatDocumentHandler *previousDocumentHandler);

    ChatDocumentHandler *nextDocumentHandler() const;
    void setNextDocumentHandler(ChatDocumentHandler *nextDocumentHandler);

    QString fixedStartChars() const;
    void setFixedStartChars(const QString &chars);
    QString fixedEndChars() const;
    void setFixedEndChars(const QString &chars);
    QString initialText() const;
    void setInitialText(const QString &text);

    bool isEmpty() const;
    bool atFirstLine() const;
    bool atLastLine() const;
    void setCursorFromDocumentHandler(ChatDocumentHandler *previousDocumentHandler, bool infront, int defaultPosition = 0);
    int lineCount() const;
    std::optional<int> lineLength(int lineNumber) const;
    int cursorPositionInLine() const;
    QTextDocumentFragment takeFirstBlock();
    void fillFragments(bool &hasBefore, QTextDocumentFragment &midFragment, std::optional<QTextDocumentFragment> &afterFragment);

    /**
     * @brief Update the mentions in @p document when editing a message.
     */
    Q_INVOKABLE void updateMentions(const QString &editId);

    Q_INVOKABLE void tab();
    Q_INVOKABLE void deleteChar();
    Q_INVOKABLE void backspace();
    Q_INVOKABLE void insertReturn();
    void insertFragment(const QTextDocumentFragment fragment, InsertPosition position = Cursor, bool keepPosition = false);
    Q_INVOKABLE void insertCompletion(const QString &text, const QUrl &link);

    Q_INVOKABLE void dumpHtml();
    Q_INVOKABLE QString htmlText() const;

Q_SIGNALS:
    void typeChanged();
    void textItemChanged();
    void roomChanged();

    void atFirstLineChanged();
    void atLastLineChanged();

    void currentListStyleChanged();

    void formatChanged();
    void textFormatChanged();
    void styleChanged();

    void contentsChanged();

    void unhandledBackspaceAtBeginning(ChatDocumentHandler *self);
    void removeMe(ChatDocumentHandler *self);

private:
    ChatBarType::Type m_type = ChatBarType::None;
    QPointer<NeoChatRoom> m_room;
    QPointer<QmlTextItemWrapper> m_textItem;
    void connectTextItem();

    QPointer<ChatDocumentHandler> m_previousDocumentHandler;
    QPointer<ChatDocumentHandler> m_nextDocumentHandler;

    QString m_fixedStartChars = {};
    QString m_fixedEndChars = {};
    QString m_initialText = {};
    void initializeChars();

    SyntaxHighlighter *m_highlighter = nullptr;

    QString getText() const;
    void pushMention(const Mention mention) const;

    std::optional<Qt::TextFormat> textFormat() const;
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    QString trim(QString string) const;
};
