// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QTextDocumentFragment>

#include "enums/chatbartype.h"
#include "enums/richformat.h"
#include "nestedlisthelper_p.h"

class QTextDocument;

class ChatBarSyntaxHighlighter;
class NeoChatRoom;

/**
 * @class ChatTextItemHelper
 *
 * A class to wrap around a QQuickItem that is a QML TextEdit (or inherited from it).
 *
 * This class has 2 key functions:
 *  - Provide easy read/write access to the properties of the TextEdit. This is required
 *    because Qt does not give us access to the cpp headers of most QML items.
 *  - Provide standard functions to edit the underlying QTextDocument.
 *
 * @sa QQuickItem, TextEdit, QTextDocument
 */
class ChatTextItemHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QML text Item the ChatTextItemHelper is handling.
     */
    Q_PROPERTY(QQuickItem *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    /**
     * @brief The QML text Item the ChatTextItemHelper is handling.
     */
    Q_PROPERTY(QRect cursorRectangle READ cursorRectangle NOTIFY cursorPositionChanged)

public:
    enum InsertPosition {
        Cursor,
        Start,
        End,
    };

    explicit ChatTextItemHelper(QObject *parent = nullptr);

    /**
     * @brief Get the NeoChatRoom used by the syntax highlighter.
     *
     * @sa NeoChatRoom
     */
    NeoChatRoom *room() const;

    /**
     * @brief Set the NeoChatRoom required by the syntax highlighter.
     *
     * @sa NeoChatRoom
     */
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the ChatBarType::Type used by the syntax highlighter.
     *
     * @sa ChatBarType::Type
     */
    ChatBarType::Type type() const;

    /**
     * @brief Set the ChatBarType::Type required by the syntax highlighter.
     *
     * @sa ChatBarType::Type
     */
    void setType(ChatBarType::Type type);

    QQuickItem *textItem() const;
    void setTextItem(QQuickItem *textItem);

    /**
     * @brief The text format of the wrapped item.
     */
    std::optional<Qt::TextFormat> textFormat() const;

    /**
     * @brief Whether a completion has started based on recent text entry.
     */
    bool isCompleting = false;

    /**
     * @brief The fixed characters that will always be at the beginning of the text item.
     */
    QString fixedStartChars() const;

    /**
     * @brief The fixed characters that will always be at the end of the text item.
     */
    QString fixedEndChars() const;

    /**
     * @brief Set the fixed characters that will always be at the beginning and end of the text item.
     */
    void setFixedChars(const QString &startChars, const QString &endChars);

    /**
     * @brief Any QTextDocumentFragment to initialise the text item with when set.
     */
    QTextDocumentFragment initialFragment() const;

    /**
     * @brief Set the QTextDocumentFragment to initialise the text item with when set.
     *
     * This text will only be set if the text item is empty when set.
     */
    void setInitialFragment(const QTextDocumentFragment &fragment);

    /**
     * @brief The underlying QTextDocument.
     *
     * @sa QTextDocument
     */
    QTextDocument *document() const;

    /**
     * @brief Whetehr the underlying QTextDocument is empty.
     *
     * @sa QTextDocument
     */
    bool isEmpty() const;

    /**
     * @brief The line count of the text item.
     */
    int lineCount() const;

    /**
     * @brief Remove the first QTextBlock from the QTextDocument and return as a QTextDocumentFragment.
     *
     * @sa QTextBlock, QTextDocument, QTextDocumentFragment
     */
    QTextDocumentFragment takeFirstBlock();

    /**
     * @brief Fill the given QTextDocumentFragment with the text item contents.
     *
     * The idea is to split the QTextDocument into 3. There is the QTextBlock that the
     * cursor is currently in, the midFragment. Then if there are any blocks after
     * this they are put into the afterFragment. The if there is any block before
     * the midFragment these are left and hasBefore is set to true.
     *
     * This is used when inserting a new block type at the cursor. The midFragement will be
     * given the new style and then the before and after are put back as the same
     * block type.
     *
     * @sa QTextBlock, QTextDocument, QTextDocumentFragment
     */
    void fillFragments(bool &hasBefore, QTextDocumentFragment &midFragment, std::optional<QTextDocumentFragment> &afterFragment);

    /**
     * @brief Insert the given QTextDocumentFragment as the given position.
     */
    void insertFragment(const QTextDocumentFragment fragment, InsertPosition position = Cursor, bool keepPosition = false);

    /**
     * @brief Return a QTextCursor pointing to the current cursor position.
     */
    QTextCursor textCursor() const;

    /**
     * @brief Return the current cursor position of the underlying text item.
     */
    std::optional<int> cursorPosition() const;

    /**
     * @brief Return the rectangle where the cursor of the underlying text item is rendered.
     */
    QRect cursorRectangle() const;

    /**
     * @brief Set the cursor position of the underlying text item to the given value.
     */
    void setCursorPosition(int pos);

    /**
     * @brief Set the selection of the underlying text item to the given cursor.
     */
    void setSelection(const QTextCursor &cursor);

    /**
     * @brief Set the cursor visibility of the underlying text item to the given value.
     */
    void setCursorVisible(bool visible);

    /**
     * @brief Set the cursor position to the same as the given text item.
     *
     * This will either be the first or last line dependent upon the infront value.
     */
    void setCursorFromTextItem(ChatTextItemHelper *textItem, bool infront);

    /**
     * @brief Merge the given format on the given QTextCursor.
     */
    void mergeFormatOnCursor(RichFormat::Format format, QTextCursor cursor = {});

    /**
     * @brief Whether the list can be indented more at the given cursor.
     */
    bool canIndentListMoreAtCursor(QTextCursor cursor = {}) const;

    /**
     * @brief Whether the list can be indented less at the given cursor.
     */
    bool canIndentListLessAtCursor(QTextCursor cursor = {}) const;

    /**
     * @brief Indented the list more at the given cursor.
     */
    void indentListMoreAtCursor(QTextCursor cursor = {});

    /**
     * @brief Indented the list less at the given cursor.
     */
    void indentListLessAtCursor(QTextCursor cursor = {});

    /**
     * @brief Force active focus on the underlying text item.
     */
    void forceActiveFocus() const;

    /**
     * @brief Rehightlight the text in the text item.
     */
    void rehighlight() const;

    /**
     * @brief Whether there is any rich formatting in the item text.
     */
    bool hasRichFormatting() const;

    /**
     * @brief Output the text in the text item in markdown format.
     */
    QString markdownText() const;

    /**
     * @brief Output the text in the text item in plain text format.
     */
    QString plainText() const;

    /**
     * @brief Output the text in the text item as a QTextDocumentFragment.
     */
    QTextDocumentFragment toFragment() const;

Q_SIGNALS:
    /**
     * @brief Emitted when the NeoChatRoom used by the syntax highlighter is changed.
     */
    void roomChanged();

    /**
     * @brief Emitted when the ChatBarType::Type used by the syntax highlighter is changed.
     */
    void typeChanged();

    void textItemChanged();

    /**
     * @brief Emitted when the textFormat property of the underlying text item is changed.
     */
    void textFormatChanged();

    /**
     * @brief Emitted when the char format at the current cursor is changed.
     */
    void charFormatChanged();

    /**
     * @brief Emitted when the style at the current cursor is changed.
     */
    void styleChanged();

    /**
     * @brief Emitted when the list format at the current cursor is changed.
     */
    void listChanged();

    /**
     * @brief Emitted when the contents of the underlying text item are changed.
     */
    void contentsChange(int position, int charsRemoved, int charsAdded);

    /**
     * @brief Emitted when the contents of the underlying text item are changed.
     */
    void contentsChanged();

    /**
     * @brief Emitted when the contents of the underlying text item are cleared.
     */
    void cleared(ChatTextItemHelper *self);

    /**
     * @brief Emitted when the cursor position of the underlying text item is changed.
     */
    void cursorPositionChanged(bool fromContentsChange);

private:
    QPointer<QQuickItem> m_textItem;
    QPointer<ChatBarSyntaxHighlighter> m_highlighter;

    bool m_contentsJustChanged = false;

    QString m_fixedStartChars = {};
    QString m_fixedEndChars = {};
    QTextDocumentFragment m_initialFragment = {};
    void initialize();
    bool m_initializingChars = false;

    std::optional<int> lineLength(int lineNumber) const;

    int selectionStart() const;
    int selectionEnd() const;

    void mergeTextFormatOnCursor(RichFormat::Format format, QTextCursor cursor);
    void mergeStyleFormatOnCursor(RichFormat::Format format, QTextCursor cursor);
    void mergeListFormatOnCursor(RichFormat::Format format, const QTextCursor &cursor);
    NestedListHelper m_nestedListHelper;

    QString trim(QString string) const;

private Q_SLOTS:
    void itemTextFormatChanged();
    void itemCursorPositionChanged();
};
