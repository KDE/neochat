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
#include "enums/chatbartype.h"
#include "enums/textstyle.h"
#include "models/completionmodel.h"
#include "neochatroom.h"
#include "nestedlisthelper_p.h"

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
     * @brief The current room that the text document is being handled for.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

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
     * @brief Whether the cursor is cuurently on the first line.
     */
    Q_PROPERTY(bool atFirstLine READ atFirstLine NOTIFY atFirstLineChanged)

    /**
     * @brief Whether the cursor is cuurently on the last line.
     */
    Q_PROPERTY(bool atLastLine READ atLastLine NOTIFY atLastLineChanged)

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY formatChanged)
    Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY formatChanged)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline NOTIFY formatChanged)
    Q_PROPERTY(bool strikethrough READ strikethrough WRITE setStrikethrough NOTIFY formatChanged)

    Q_PROPERTY(TextStyle::Style style READ style WRITE setStyle NOTIFY styleChanged)

    // Q_PROPERTY(bool canIndentList READ canIndentList NOTIFY cursorPositionChanged)
    // Q_PROPERTY(bool canDedentList READ canDedentList NOTIFY cursorPositionChanged)
    Q_PROPERTY(int currentListStyle READ currentListStyle NOTIFY currentListStyleChanged)
    // Q_PROPERTY(int currentHeadingLevel READ currentHeadingLevel NOTIFY cursorPositionChanged)

    // Q_PROPERTY(bool list READ list WRITE setList NOTIFY listChanged)

    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileUrlChanged)
    Q_PROPERTY(QString fileType READ fileType NOTIFY fileUrlChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY fileUrlChanged)

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

    Q_INVOKABLE void complete(int index);

    CompletionModel *completionModel() const;

    /**
     * @brief Update the mentions in @p document when editing a message.
     */
    Q_INVOKABLE void updateMentions(const QString &editId);

    QString fontFamily() const;
    void setFontFamily(const QString &family);

    QColor textColor() const;
    void setTextColor(const QColor &color);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    bool bold() const;
    void setBold(bool bold);
    bool italic() const;
    void setItalic(bool italic);

    bool underline() const;
    void setUnderline(bool underline);

    bool strikethrough() const;
    void setStrikethrough(bool strikethrough);

    bool canIndentList() const;
    bool canDedentList() const;
    int currentListStyle() const;

    TextStyle::Style style() const;
    void setStyle(TextStyle::Style style);

    // bool list() const;
    // void setList(bool list);

    int fontSize() const;
    void setFontSize(int size);

    QString fileName() const;
    QString fileType() const;
    QUrl fileUrl() const;

    Q_INVOKABLE void deleteChar();
    Q_INVOKABLE void backspace();
    Q_INVOKABLE void insertText(const QString &text);
    void insertFragment(const QTextDocumentFragment fragment, InsertPosition position = Cursor, bool keepPosition = false);
    Q_INVOKABLE QString currentLinkUrl() const;
    Q_INVOKABLE QString currentLinkText() const;
    Q_INVOKABLE void updateLink(const QString &linkUrl, const QString &linkText);
    Q_INVOKABLE void insertImage(const QUrl &imagePath);
    Q_INVOKABLE void insertTable(int rows, int columns);

    Q_INVOKABLE void indentListLess();
    Q_INVOKABLE void indentListMore();

    Q_INVOKABLE void setListStyle(int styleIndex);

    Q_INVOKABLE void dumpHtml();
    Q_INVOKABLE QString htmlText() const;

Q_SIGNALS:
    void typeChanged();
    void textItemChanged();
    void roomChanged();

    void atFirstLineChanged();
    void atLastLineChanged();

    void fontFamilyChanged();
    void textColorChanged();
    void alignmentChanged();

    void boldChanged();
    void italicChanged();
    void underlineChanged();
    void checkableChanged();
    void strikethroughChanged();
    void currentListStyleChanged();
    void fontSizeChanged();
    void fileUrlChanged();

    void formatChanged();
    void styleChanged();

    void contentsChanged();

    void unhandledBackspaceAtBeginning(ChatDocumentHandler *self);
    void removeMe(ChatDocumentHandler *self);

private:
    ChatBarType::Type m_type = ChatBarType::None;
    QPointer<QQuickItem> m_textItem;
    QTextDocument *document() const;

    QPointer<ChatDocumentHandler> m_previousDocumentHandler;
    QPointer<ChatDocumentHandler> m_nextDocumentHandler;

    QString m_fixedStartChars = {};
    QString m_fixedEndChars = {};
    QString m_initialText = {};
    void initializeChars();

    int completionStartIndex() const;

    QPointer<NeoChatRoom> m_room;

    int cursorPosition() const;
    int selectionStart() const;
    int selectionEnd() const;

    QString getText() const;
    void pushMention(const Mention mention) const;

    SyntaxHighlighter *m_highlighter = nullptr;
    QQuickItem *m_textArea;

    CompletionModel *m_completionModel = nullptr;
    QTextCursor textCursor() const;
    std::optional<Qt::TextFormat> textFormat() const;
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void selectLinkText(QTextCursor *cursor) const;
    NestedListHelper m_nestedListHelper;
    QColor linkColor();
    QColor mLinkColor;
    void regenerateColorScheme();
    QUrl m_fileUrl;

    QString trim(QString string) const;

private Q_SLOTS:
    void updateCursor();
};
