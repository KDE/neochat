// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbuttonhelper.h"

#include <Kirigami/Platform/PlatformTheme>

#include "chatbarcache.h"
#include "chattextitemhelper.h"
#include "enums/chatbartype.h"
#include "enums/richformat.h"
#include "neochatroom.h"

ChatButtonHelper::ChatButtonHelper(QObject *parent)
    : QObject(parent)
{
}

ChatTextItemHelper *ChatButtonHelper::textItem() const
{
    return m_textItem;
}

void ChatButtonHelper::setTextItem(ChatTextItemHelper *textItem)
{
    if (textItem == m_textItem) {
        return;
    }

    if (m_textItem) {
        m_textItem->disconnect(this);
    }

    m_textItem = textItem;

    if (m_textItem) {
        connect(m_textItem, &ChatTextItemHelper::roomChanged, this, [this]() {
            if (m_textItem->room() && m_textItem->type() != ChatBarType::None) {
                const auto cache = m_textItem->room()->cacheForType(m_textItem->type());
                connect(cache, &ChatBarCache::attachmentPathChanged, this, &ChatButtonHelper::richFormatEnabledChanged);
            }
        });
        connect(m_textItem, &ChatTextItemHelper::textFormatChanged, this, &ChatButtonHelper::richFormatEnabledChanged);
        connect(m_textItem, &ChatTextItemHelper::textFormatChanged, this, &ChatButtonHelper::styleChanged);
        connect(m_textItem, &ChatTextItemHelper::charFormatChanged, this, &ChatButtonHelper::charFormatChanged);
        connect(m_textItem, &ChatTextItemHelper::styleChanged, this, &ChatButtonHelper::styleChanged);
        connect(m_textItem, &ChatTextItemHelper::listChanged, this, &ChatButtonHelper::listChanged);
    }

    Q_EMIT textItemChanged();
    Q_EMIT richFormatEnabledChanged();
    Q_EMIT styleChanged();
}

bool ChatButtonHelper::inQuote() const
{
    return m_inQuote;
}

void ChatButtonHelper::setInQuote(bool inQuote)
{
    if (inQuote == m_inQuote) {
        return;
    }
    m_inQuote = inQuote;
    Q_EMIT inQuoteChanged();
    Q_EMIT richFormatEnabledChanged();
    Q_EMIT styleChanged();
}

bool ChatButtonHelper::richFormatEnabled() const
{
    if (!m_textItem) {
        return false;
    }
    const auto styleAvailable = styleFormatEnabled();
    if (!styleAvailable) {
        return false;
    }
    const auto format = m_textItem->textFormat();
    if (format) {
        return format != Qt::PlainText;
    }
    return false;
}

bool ChatButtonHelper::styleFormatEnabled() const
{
    if (!m_textItem) {
        return false;
    }
    const auto room = m_textItem->room();
    if (!room) {
        return false;
    }
    if (const auto cache = room->cacheForType(m_textItem->type())) {
        return cache->attachmentPath().isEmpty();
    }
    return true;
}

bool ChatButtonHelper::bold() const
{
    if (!m_textItem) {
        return false;
    }
    const auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return RichFormat::formatsAtCursor(cursor).contains(RichFormat::Bold);
}

bool ChatButtonHelper::italic() const
{
    if (!m_textItem) {
        return false;
    }
    const auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return RichFormat::formatsAtCursor(cursor).contains(RichFormat::Italic);
}

bool ChatButtonHelper::underline() const
{
    if (!m_textItem) {
        return false;
    }
    const auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return RichFormat::formatsAtCursor(cursor).contains(RichFormat::Underline);
}

bool ChatButtonHelper::strikethrough() const
{
    if (!m_textItem) {
        return false;
    }
    const auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return RichFormat::formatsAtCursor(cursor).contains(RichFormat::Strikethrough);
}

bool ChatButtonHelper::unorderedList() const
{
    if (!m_textItem) {
        return false;
    }
    const auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return RichFormat::formatsAtCursor(cursor).contains(RichFormat::UnorderedList);
}

bool ChatButtonHelper::orderedList() const
{
    if (!m_textItem) {
        return false;
    }
    const auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return RichFormat::formatsAtCursor(cursor).contains(RichFormat::OrderedList);
}

RichFormat::Format ChatButtonHelper::currentStyle() const
{
    if (!m_textItem) {
        return RichFormat::Paragraph;
    }
    if (!richFormatEnabled()) {
        return RichFormat::Code;
    }
    const auto currentHeadingLevel = m_textItem->textCursor().blockFormat().headingLevel();
    if (currentHeadingLevel == 0 && m_inQuote) {
        return RichFormat::Quote;
    }
    return static_cast<RichFormat::Format>(currentHeadingLevel);
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
