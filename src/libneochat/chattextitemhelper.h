// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>
#include <qcontainerfwd.h>

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
 * @note This basically exists because Qt does not give us access to the cpp headers of
 * most QML items.
 *
 * @sa QQuickItem, TextEdit
 */
class ChatTextItemHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QML text Item the ChatTextItemHelper is handling.
     */
    Q_PROPERTY(QQuickItem *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

public:
    enum InsertPosition {
        Cursor,
        Start,
        End,
    };

    explicit ChatTextItemHelper(QObject *parent = nullptr);

    void setRoom(NeoChatRoom *room);

    void setType(ChatBarType::Type type);

    QQuickItem *textItem() const;
    void setTextItem(QQuickItem *textItem);

    QString fixedStartChars() const;
    QString fixedEndChars() const;
    void setFixedChars(const QString &startChars, const QString &endChars);
    QString initialText() const;
    void setInitialText(const QString &text);

    QTextDocument *document() const;
    int lineCount() const;
    QTextDocumentFragment takeFirstBlock();
    void fillFragments(bool &hasBefore, QTextDocumentFragment &midFragment, std::optional<QTextDocumentFragment> &afterFragment);
    void insertFragment(const QTextDocumentFragment fragment, InsertPosition position = Cursor, bool keepPosition = false);

    QTextCursor textCursor() const;
    int cursorPosition() const;
    void setCursorPosition(int pos);
    void setCursorVisible(bool visible);
    void setCursorFromTextItem(ChatTextItemHelper *textItem, bool infront, int defaultPosition = 0);

    QList<RichFormat::Format> formatsAtCursor(QTextCursor cursor = {}) const;
    void mergeFormatOnCursor(RichFormat::Format format, QTextCursor cursor = {});

    bool canIndentListMoreAtCursor(QTextCursor cursor = {}) const;
    bool canIndentListLessAtCursor(QTextCursor cursor = {}) const;
    void indentListMoreAtCursor(QTextCursor cursor = {});
    void indentListLessAtCursor(QTextCursor cursor = {});

    void forceActiveFocus() const;

    void rehighlight() const;

    QString markdownText() const;

Q_SIGNALS:
    void textItemChanged();

    void contentsChange(int position, int charsRemoved, int charsAdded);

    void contentsChanged();

    void cleared(ChatTextItemHelper *self);

    void cursorPositionChanged();

    void formatChanged();
    void textFormatChanged();
    void styleChanged();
    void listChanged();

private:
    QPointer<QQuickItem> m_textItem;
    QPointer<ChatBarSyntaxHighlighter> m_highlighter;

    std::optional<Qt::TextFormat> textFormat() const;

    QString m_fixedStartChars = {};
    QString m_fixedEndChars = {};
    QString m_initialText = {};
    void initializeChars();

    bool isEmpty() const;
    std::optional<int> lineLength(int lineNumber) const;

    int selectionStart() const;
    int selectionEnd() const;

    void mergeTextFormatOnCursor(RichFormat::Format format, QTextCursor cursor);
    void mergeStyleFormatOnCursor(RichFormat::Format format, QTextCursor cursor);
    void mergeListFormatOnCursor(RichFormat::Format format, const QTextCursor &cursor);
    NestedListHelper m_nestedListHelper;

    QString trim(QString string) const;

private Q_SLOTS:
    void itemCursorPositionChanged();
};
