// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>

class QTextDocument;

/**
 * @class QmlTextItemWrapper
 *
 * A class to wrap around a QQuickItem that is a QML TextEdit (or inherited from it) and provide easy acess to its properties.
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
    void setCursorPosition(int pos);
    void setCursorVisible(bool visible);

    void forceActiveFocus() const;

Q_SIGNALS:
    void textItemChanged();

    void textDocumentContentsChange(int position, int charsRemoved, int charsAdded);

    void textDocumentContentsChanged();

    void textDocumentCursorPositionChanged();

private:
    QPointer<QQuickItem> m_textItem;

    int cursorPosition() const;
    int selectionStart() const;
    int selectionEnd() const;

private Q_SLOTS:
    void textDocCursorChanged();
};
