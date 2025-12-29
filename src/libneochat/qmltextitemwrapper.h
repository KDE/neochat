// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>

#include "enums/richformat.h"
#include "nestedlisthelper_p.h"

class QTextDocument;

/**
 * @class QmlTextItemWrapper
 *
 * A class to wrap around a QQuickItem that is a QML TextEdit (or inherited from it).
 *
 * @note This basically exists because Qt does not give us access to the cpp headers of
 * most QML items.
 *
 * @sa QQuickItem, TextEdit
 */
class QmlTextItemWrapper : public QObject
{
    Q_OBJECT

public:
    explicit QmlTextItemWrapper(QObject *parent);

    QQuickItem *textItem() const;
    void setTextItem(QQuickItem *textItem);

    QTextDocument *document() const;

    QTextCursor textCursor() const;
    int cursorPosition() const;
    void setCursorPosition(int pos);
    void setCursorVisible(bool visible);

    QList<RichFormat::Format> formatsAtCursor(QTextCursor cursor = {}) const;
    void mergeFormatOnCursor(RichFormat::Format format, QTextCursor cursor = {});

    bool canIndentListMoreAtCursor(QTextCursor cursor = {}) const;
    bool canIndentListLessAtCursor(QTextCursor cursor = {}) const;
    void indentListMoreAtCursor(QTextCursor cursor = {});
    void indentListLessAtCursor(QTextCursor cursor = {});

    void forceActiveFocus() const;

Q_SIGNALS:
    void textItemChanged();

    void contentsChange(int position, int charsRemoved, int charsAdded);

    void contentsChanged();

    void cursorPositionChanged();

    void formatChanged();
    void textFormatChanged();
    void styleChanged();
    void listChanged();

private:
    QPointer<QQuickItem> m_textItem;

    int selectionStart() const;
    int selectionEnd() const;

    void mergeTextFormatOnCursor(RichFormat::Format format, QTextCursor cursor);
    void mergeStyleFormatOnCursor(RichFormat::Format format, QTextCursor cursor);
    void mergeListFormatOnCursor(RichFormat::Format format, const QTextCursor &cursor);
    NestedListHelper m_nestedListHelper;

private Q_SLOTS:
    void itemCursorPositionChanged();
};
