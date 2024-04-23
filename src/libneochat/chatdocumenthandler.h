// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QTextCursor>

#include "chatbarcache.h"
#include "models/completionmodel.h"
#include "neochatroom.h"
#include "nestedlisthelper_p.h"

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
    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)

    Q_PROPERTY(QQuickItem *textArea READ textArea WRITE setTextArea NOTIFY textAreaChanged)

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
    Q_PROPERTY(CompletionModel *completionModel READ completionModel CONSTANT)

    /**
     * @brief The current room that the text document is being handled for.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief The cache for the chat bar the text document is being handled for.
     */
    Q_PROPERTY(ChatBarCache *chatBarCache READ chatBarCache WRITE setChatBarCache NOTIFY chatBarCacheChanged)

    /**
     * @brief The color to highlight user mentions.
     */
    Q_PROPERTY(QColor mentionColor READ mentionColor WRITE setMentionColor NOTIFY mentionColorChanged)

    /**
     * @brief The color to highlight spelling errors.
     */
    Q_PROPERTY(QColor errorColor READ errorColor WRITE setErrorColor NOTIFY errorColorChanged)

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY boldChanged)
    Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY italicChanged)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline NOTIFY underlineChanged)
    Q_PROPERTY(bool strikethrough READ strikethrough WRITE setStrikethrough NOTIFY strikethroughChanged)

    Q_PROPERTY(bool canIndentList READ canIndentList NOTIFY cursorPositionChanged)
    Q_PROPERTY(bool canDedentList READ canDedentList NOTIFY cursorPositionChanged)
    Q_PROPERTY(int currentListStyle READ currentListStyle NOTIFY currentListStyleChanged)
    Q_PROPERTY(int currentHeadingLevel READ currentHeadingLevel NOTIFY cursorPositionChanged)

    // Q_PROPERTY(bool list READ list WRITE setList NOTIFY listChanged)

    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileUrlChanged)
    Q_PROPERTY(QString fileType READ fileType NOTIFY fileUrlChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY fileUrlChanged)

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

    [[nodiscard]] ChatBarCache *chatBarCache() const;
    void setChatBarCache(ChatBarCache *chatBarCache);

    Q_INVOKABLE void complete(int index);

    CompletionModel *completionModel() const;

    [[nodiscard]] QColor mentionColor() const;
    void setMentionColor(const QColor &color);

    [[nodiscard]] QColor errorColor() const;
    void setErrorColor(const QColor &color);

    QQuickItem *textArea() const;
    void setTextArea(QQuickItem *textArea);

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

    int currentHeadingLevel() const;

    // bool list() const;
    // void setList(bool list);

    int fontSize() const;
    void setFontSize(int size);

    QString fileName() const;
    QString fileType() const;
    QUrl fileUrl() const;

    Q_INVOKABLE void insertText(const QString &text);
    Q_INVOKABLE QString currentLinkUrl() const;
    Q_INVOKABLE QString currentLinkText() const;
    Q_INVOKABLE void updateLink(const QString &linkUrl, const QString &linkText);
    Q_INVOKABLE void insertImage(const QUrl &imagePath);
    Q_INVOKABLE void insertTable(int rows, int columns);

    Q_INVOKABLE void indentListLess();
    Q_INVOKABLE void indentListMore();

    Q_INVOKABLE void setListStyle(int styleIndex);
    Q_INVOKABLE void setHeadingLevel(int level);

    Q_INVOKABLE void dumpHtml();
    Q_INVOKABLE QString htmlText();

Q_SIGNALS:
    void documentChanged();
    void cursorPositionChanged();
    void roomChanged();
    void chatBarCacheChanged();
    void selectionStartChanged();
    void selectionEndChanged();
    void textAreaChanged();
    void errorColorChanged();
    void mentionColorChanged();

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

private:
    int completionStartIndex() const;

    QPointer<QQuickTextDocument> m_document;

    QPointer<NeoChatRoom> m_room;
    QPointer<ChatBarCache> m_chatBarCache;

    QColor m_mentionColor;
    QColor m_errorColor;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    QString getText() const;
    void pushMention(const Mention mention) const;

    SyntaxHighlighter *m_highlighter = nullptr;
    QQuickItem *m_textArea;

    CompletionModel *m_completionModel = nullptr;
    QTextCursor textCursor() const;
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void selectLinkText(QTextCursor *cursor) const;
    NestedListHelper m_nestedListHelper;
    QColor linkColor();
    QColor mLinkColor;
    void regenerateColorScheme();
    QUrl m_fileUrl;
};
