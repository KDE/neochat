// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "qmltextitemwrapper.h"

#include <QQuickTextDocument>
#include <QTextCursor>

QmlTextItemWrapper::QmlTextItemWrapper(QObject *parent)
    : QObject(parent)
{
}

QQuickItem *QmlTextItemWrapper::textItem() const
{
    return m_textItem;
}

void QmlTextItemWrapper::setTextItem(QQuickItem *textItem)
{
    if (textItem == m_textItem) {
        return;
    }

    if (m_textItem) {
        m_textItem->disconnect(this);
        if (const auto textDoc = document()) {
            textDoc->disconnect(this);
        }
    }

    m_textItem = textItem;

    if (m_textItem) {
        connect(m_textItem, SIGNAL(cursorPositionChanged()), this, SLOT(textDocCursorChanged()));
        if (document()) {
            connect(document(), &QTextDocument::contentsChanged, this, &QmlTextItemWrapper::textDocumentContentsChanged);
            connect(document(), &QTextDocument::contentsChange, this, &QmlTextItemWrapper::textDocumentContentsChange);
        }
    }

    Q_EMIT textItemChanged();
}

QTextDocument *QmlTextItemWrapper::document() const
{
    if (!m_textItem) {
        return nullptr;
    }
    const auto quickDocument = qvariant_cast<QQuickTextDocument *>(textItem()->property("textDocument"));
    return quickDocument ? quickDocument->textDocument() : nullptr;
}

int QmlTextItemWrapper::cursorPosition() const
{
    if (!m_textItem) {
        return -1;
    }
    return m_textItem->property("cursorPosition").toInt();
}

int QmlTextItemWrapper::selectionStart() const
{
    if (!m_textItem) {
        return -1;
    }
    return m_textItem->property("selectionStart").toInt();
}

int QmlTextItemWrapper::selectionEnd() const
{
    if (!m_textItem) {
        return -1;
    }
    return m_textItem->property("selectionEnd").toInt();
}

QTextCursor QmlTextItemWrapper::textCursor() const
{
    if (!document()) {
        return QTextCursor();
    }

    QTextCursor cursor = QTextCursor(document());
    if (selectionStart() != selectionEnd()) {
        cursor.setPosition(selectionStart());
        cursor.setPosition(selectionEnd(), QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(cursorPosition());
    }
    return cursor;
}

void QmlTextItemWrapper::setCursorPosition(int pos)
{
    if (!m_textItem) {
        return;
    }
    m_textItem->setProperty("cursorPosition", pos);
}

void QmlTextItemWrapper::setCursorVisible(bool visible)
{
    if (!m_textItem) {
        return;
    }
    m_textItem->setProperty("cursorVisible", visible);
}

void QmlTextItemWrapper::textDocCursorChanged()
{
    Q_EMIT textDocumentCursorPositionChanged();
}

void QmlTextItemWrapper::forceActiveFocus() const
{
    if (!m_textItem) {
        return;
    }
    m_textItem->forceActiveFocus();
}

#include "moc_qmltextitemwrapper.cpp"
