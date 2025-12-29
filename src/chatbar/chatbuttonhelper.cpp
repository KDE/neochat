// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbuttonhelper.h"

#include <Kirigami/Platform/PlatformTheme>

#include "enums/richformat.h"

ChatButtonHelper::ChatButtonHelper(QObject *parent)
    : QObject(parent)
{
}

QmlTextItemWrapper *ChatButtonHelper::textItem() const
{
    return m_textItem;
}

void ChatButtonHelper::setTextItem(QmlTextItemWrapper *textItem)
{
    if (textItem == m_textItem) {
        return;
    }

    if (m_textItem) {
        m_textItem->disconnect(this);
    }

    m_textItem = textItem;

    if (m_textItem) {
        connect(m_textItem, &QmlTextItemWrapper::textItemChanged, this, &ChatButtonHelper::textItemChanged);
        connect(m_textItem, &QmlTextItemWrapper::formatChanged, this, &ChatButtonHelper::linkChanged);
        connect(m_textItem, &QmlTextItemWrapper::textFormatChanged, this, &ChatButtonHelper::textFormatChanged);
        connect(m_textItem, &QmlTextItemWrapper::styleChanged, this, &ChatButtonHelper::styleChanged);
        connect(m_textItem, &QmlTextItemWrapper::listChanged, this, &ChatButtonHelper::listChanged);
    }

    Q_EMIT textItemChanged();
}

bool ChatButtonHelper::bold() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->formatsAtCursor().contains(RichFormat::Bold);
}

bool ChatButtonHelper::italic() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->formatsAtCursor().contains(RichFormat::Italic);
}

bool ChatButtonHelper::underline() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->formatsAtCursor().contains(RichFormat::Underline);
}

bool ChatButtonHelper::strikethrough() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->formatsAtCursor().contains(RichFormat::Strikethrough);
}

bool ChatButtonHelper::unorderedList() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->formatsAtCursor().contains(RichFormat::UnorderedList);
}

bool ChatButtonHelper::orderedList() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->formatsAtCursor().contains(RichFormat::OrderedList);
}

void ChatButtonHelper::setFormat(RichFormat::Format format)
{
    if (!m_textItem) {
        return;
    }
    m_textItem->mergeFormatOnCursor(format);
}

bool ChatButtonHelper::canIndentListMore() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->canIndentListMoreAtCursor();
}

bool ChatButtonHelper::canIndentListLess() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->canIndentListLessAtCursor();
}

void ChatButtonHelper::indentListMore()
{
    if (!m_textItem) {
        return;
    }
    m_textItem->indentListMoreAtCursor();
}

void ChatButtonHelper::indentListLess()
{
    if (!m_textItem) {
        return;
    }
    m_textItem->indentListLessAtCursor();
}

void ChatButtonHelper::insertText(const QString &text)
{
    if (!m_textItem) {
        return;
    }
    m_textItem->textCursor().insertText(text);
}

QString ChatButtonHelper::currentLinkUrl() const
{
    if (!m_textItem) {
        return {};
    }
    return m_textItem->textCursor().charFormat().anchorHref();
}

void ChatButtonHelper::selectLinkText(QTextCursor &cursor) const
{
    // If the cursor is on a link, select the text of the link.
    if (cursor.charFormat().isAnchor()) {
        const QString aHref = cursor.charFormat().anchorHref();

        // Move cursor to start of link
        while (cursor.charFormat().anchorHref() == aHref) {
            if (cursor.atStart()) {
                break;
            }
            cursor.setPosition(cursor.position() - 1);
        }
        if (cursor.charFormat().anchorHref() != aHref) {
            cursor.setPosition(cursor.position() + 1, QTextCursor::KeepAnchor);
        }

        // Move selection to the end of the link
        while (cursor.charFormat().anchorHref() == aHref) {
            if (cursor.atEnd()) {
                break;
            }
            const int oldPosition = cursor.position();
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            // Wordaround Qt Bug. when we have a table.
            // FIXME selection url
            if (oldPosition == cursor.position()) {
                break;
            }
        }
        if (cursor.charFormat().anchorHref() != aHref) {
            cursor.setPosition(cursor.position() - 1, QTextCursor::KeepAnchor);
        }
    } else if (cursor.hasSelection()) {
        // Nothing to do. Using the currently selected text as the link text.
    } else {
        // Select current word
        cursor.movePosition(QTextCursor::StartOfWord);
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    }
}

QString ChatButtonHelper::currentLinkText() const
{
    if (!m_textItem) {
        return {};
    }
    QTextCursor cursor = m_textItem->textCursor();
    selectLinkText(cursor);
    return cursor.selectedText();
}

void ChatButtonHelper::updateLink(const QString &linkUrl, const QString &linkText)
{
    if (!m_textItem) {
        return;
    }
    auto cursor = m_textItem->textCursor();
    selectLinkText(cursor);

    cursor.beginEditBlock();

    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    const auto originalFormat = cursor.charFormat();
    auto format = cursor.charFormat();
    // Save original format to create an extra space with the existing char
    // format for the block
    if (!linkUrl.isEmpty()) {
        // Add link details
        format.setAnchor(true);
        format.setAnchorHref(linkUrl);
        // Workaround for QTBUG-1814:
        // Link formatting does not get applied immediately when setAnchor(true)
        // is called.  So the formatting needs to be applied manually.
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);

        const auto theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
        format.setUnderlineColor(theme->linkColor());
        format.setForeground(theme->linkColor());
    } else {
        // Remove link details
        format.setAnchor(false);
        format.setAnchorHref(QString());
        // Workaround for QTBUG-1814:
        // Link formatting does not get removed immediately when setAnchor(false)
        // is called. So the formatting needs to be applied manually.
        QTextDocument defaultTextDocument;
        QTextCharFormat defaultCharFormat = defaultTextDocument.begin().charFormat();

        format.setUnderlineStyle(defaultCharFormat.underlineStyle());
        format.setUnderlineColor(defaultCharFormat.underlineColor());
        format.setForeground(defaultCharFormat.foreground());
    }

    // Insert link text specified in dialog, otherwise write out url.
    QString _linkText;
    if (!linkText.isEmpty()) {
        _linkText = linkText;
    } else {
        _linkText = linkUrl;
    }
    cursor.insertText(_linkText, format);
    if (cursor.atBlockEnd()) {
        cursor.insertText(u" "_s, originalFormat);
    }
    cursor.endEditBlock();
}

#include "moc_chatbuttonhelper.cpp"
