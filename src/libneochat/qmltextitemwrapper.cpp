// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "qmltextitemwrapper.h"
#include "richformat.h"

#include <QQuickTextDocument>
#include <QTextCursor>

#include <Kirigami/Platform/PlatformTheme>

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
        connect(m_textItem, SIGNAL(cursorPositionChanged()), this, SLOT(itemCursorPositionChanged()));
        if (document()) {
            connect(document(), &QTextDocument::contentsChanged, this, &QmlTextItemWrapper::contentsChanged);
            connect(document(), &QTextDocument::contentsChange, this, &QmlTextItemWrapper::contentsChange);
        }
    }

    Q_EMIT textItemChanged();
    Q_EMIT formatChanged();
    Q_EMIT textFormatChanged();
    Q_EMIT styleChanged();
    Q_EMIT listChanged();
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

void QmlTextItemWrapper::itemCursorPositionChanged()
{
    Q_EMIT cursorPositionChanged();
    Q_EMIT formatChanged();
    Q_EMIT textFormatChanged();
    Q_EMIT styleChanged();
    Q_EMIT listChanged();
}

QList<RichFormat::Format> QmlTextItemWrapper::formatsAtCursor(QTextCursor cursor) const
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return {};
        }
    }
    return RichFormat::formatsAtCursor(cursor);
}

void QmlTextItemWrapper::mergeFormatOnCursor(RichFormat::Format format, QTextCursor cursor)
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return;
        }
    }
    switch (RichFormat::typeForFormat(format)) {
    case RichFormat::Text:
        mergeTextFormatOnCursor(format, cursor);
        return;
    case RichFormat::List:
        mergeListFormatOnCursor(format, cursor);
        return;
    case RichFormat::Block:
        if (format != RichFormat::Paragraph) {
            return;
        }
    case RichFormat::Style:
        mergeStyleFormatOnCursor(format, cursor);
        return;
    default:
        return;
    }
}

void QmlTextItemWrapper::mergeTextFormatOnCursor(RichFormat::Format format, QTextCursor cursor)
{
    if (RichFormat::typeForFormat(format) != RichFormat::Text) {
        return;
    }

    const auto theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
    const auto charFormat = RichFormat::charFormatForFormat(format, RichFormat::hasFormat(cursor, format), theme->alternateBackgroundColor());
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(charFormat);
    Q_EMIT formatChanged();
    Q_EMIT textFormatChanged();
}

void QmlTextItemWrapper::mergeStyleFormatOnCursor(RichFormat::Format format, QTextCursor cursor)
{
    // Paragraph is special because it is normally a Block format but if we're already
    // in a Paragraph it clears any existing style.
    if (!(RichFormat::typeForFormat(format) == RichFormat::Style || format == RichFormat::Paragraph)) {
        return;
    }

    cursor.beginEditBlock();
    cursor.mergeBlockFormat(RichFormat::blockFormatForFormat(format));

    // Applying style to the current line or selection
    QTextCursor selectCursor = cursor;
    if (selectCursor.hasSelection()) {
        QTextCursor top = selectCursor;
        top.setPosition(qMin(top.anchor(), top.position()));
        top.movePosition(QTextCursor::StartOfBlock);

        QTextCursor bottom = selectCursor;
        bottom.setPosition(qMax(bottom.anchor(), bottom.position()));
        bottom.movePosition(QTextCursor::EndOfBlock);

        selectCursor.setPosition(top.position(), QTextCursor::MoveAnchor);
        selectCursor.setPosition(bottom.position(), QTextCursor::KeepAnchor);
    } else {
        selectCursor.select(QTextCursor::BlockUnderCursor);
    }

    const auto chrfmt = RichFormat::charFormatForFormat(format);
    selectCursor.mergeCharFormat(chrfmt);
    cursor.mergeBlockCharFormat(chrfmt);
    cursor.endEditBlock();

    Q_EMIT formatChanged();
    Q_EMIT styleChanged();
}

void QmlTextItemWrapper::mergeListFormatOnCursor(RichFormat::Format format, const QTextCursor &cursor)
{
    m_nestedListHelper.handleOnBulletType(RichFormat::listStyleForFormat(format), cursor);
    Q_EMIT formatChanged();
    Q_EMIT listChanged();
}

bool QmlTextItemWrapper::canIndentListMoreAtCursor(QTextCursor cursor) const
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return false;
        }
    }
    return m_nestedListHelper.canIndent(cursor) && cursor.blockFormat().headingLevel() == 0;
}

bool QmlTextItemWrapper::canIndentListLessAtCursor(QTextCursor cursor) const
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return false;
        }
    }
    return m_nestedListHelper.canDedent(cursor) && cursor.blockFormat().headingLevel() == 0;
}

void QmlTextItemWrapper::indentListMoreAtCursor(QTextCursor cursor)
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return;
        }
    }
    m_nestedListHelper.handleOnIndentMore(cursor);
    Q_EMIT listChanged();
}

void QmlTextItemWrapper::indentListLessAtCursor(QTextCursor cursor)
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return;
        }
    }
    m_nestedListHelper.handleOnIndentLess(cursor);
    Q_EMIT listChanged();
}

void QmlTextItemWrapper::forceActiveFocus() const
{
    if (!m_textItem) {
        return;
    }
    m_textItem->forceActiveFocus();
}

#include "moc_qmltextitemwrapper.cpp"
