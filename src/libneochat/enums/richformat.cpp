// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "richformat.h"

#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>

#include <KLocalizedString>

QString RichFormat::styleString(Format format)
{
    switch (format) {
    case Paragraph:
        return i18nc("As in the default paragraph text style in the chat bar", "Paragraph Style");
    case Heading1:
        return i18nc("As in heading level 1 text style in the chat bar", "Heading 1");
    case Heading2:
        return i18nc("As in heading level 2 text style in the chat bar", "Heading 2");
    case Heading3:
        return i18nc("As in heading level 3 text style in the chat bar", "Heading 3");
    case Heading4:
        return i18nc("As in heading level 4 text style in the chat bar", "Heading 4");
    case Heading5:
        return i18nc("As in heading level 5 text style in the chat bar", "Heading 5");
    case Heading6:
        return i18nc("As in heading level 6 text style in the chat bar", "Heading 6");
    case Code:
        return i18nc("As in code text style in the chat bar", "Code");
    case Quote:
        return i18nc("As in quote text style in the chat bar", "\"Quote\"");
    default:
        return {};
    }
};

RichFormat::FormatType RichFormat::typeForFormat(Format format)
{
    switch (format) {
    case Code:
    case Quote:
        return Block;
    case Paragraph:
    case Heading1:
    case Heading2:
    case Heading3:
    case Heading4:
    case Heading5:
    case Heading6:
        return Style;
    case UnorderedList:
    case OrderedList:
        return List;
    default:
        return Text;
    }
};

QTextListFormat::Style RichFormat::listStyleForFormat(Format format)
{
    switch (format) {
    case UnorderedList:
        return QTextListFormat::ListDisc;
    case OrderedList:
        return QTextListFormat::ListDecimal;
    default:
        return QTextListFormat::ListStyleUndefined;
    }
}

QTextCharFormat RichFormat::charFormatForFormat(Format format, bool invert, const QColor &highlightColor)
{
    QTextCharFormat charFormat;
    if (format == Bold || headingLevelForFormat(format) > 0) {
        const auto weight = invert ? QFont::Normal : QFont::Bold;
        charFormat.setFontWeight(weight);
    }
    if (format == Italic) {
        charFormat.setFontItalic(!invert);
    }
    if (format == Underline) {
        charFormat.setFontUnderline(!invert);
    }
    if (format == Strikethrough) {
        charFormat.setFontStrikeOut(!invert);
    }
    if (format == InlineCode) {
        if (invert) {
            charFormat.setFont({});
            charFormat.setBackground({});
        } else {
            charFormat.setFontFamilies({u"monospace"_s});
            charFormat.setFontFixedPitch(!invert);
            charFormat.setBackground(highlightColor);
        }
    }
    if (headingLevelForFormat(format) > 0) {
        // Apparently, 4 is maximum for FontSizeAdjustment; otherwise level=1 and
        // level=2 look the same
        charFormat.setProperty(QTextFormat::FontSizeAdjustment, 5 - headingLevelForFormat(format));
    }
    if (format == Paragraph) {
        charFormat.setFontWeight(QFont::Normal);
        charFormat.setProperty(QTextFormat::FontSizeAdjustment, 0);
    }
    return charFormat;
}

QTextBlockFormat RichFormat::blockFormatForFormat(Format format)
{
    QTextBlockFormat blockformat;
    blockformat.setHeadingLevel(headingLevelForFormat(format));
    return blockformat;
}

int RichFormat::headingLevelForFormat(Format format)
{
    const auto intFormat = int(format);
    return intFormat <= 6 ? intFormat : 0;
}

RichFormat::Format RichFormat::formatForHeadingLevel(int level)
{
    auto clampLevel = level > 6 ? 0 : level;
    clampLevel = std::clamp(clampLevel, 0, 6);
    return static_cast<Format>(clampLevel);
}

bool RichFormat::hasFormat(QTextCursor cursor, Format format)
{
    switch (format) {
    case Paragraph:
        return cursor.blockFormat().headingLevel() == headingLevelForFormat(format);
    case Heading1:
    case Heading2:
    case Heading3:
    case Heading4:
    case Heading5:
    case Heading6:
        return cursor.blockFormat().headingLevel() == headingLevelForFormat(format) && cursor.charFormat().fontWeight() == QFont::Bold;
    case Quote:
        return cursor.blockFormat().headingLevel() == headingLevelForFormat(format) && cursor.charFormat().fontItalic();
    case Code:
        return cursor.blockFormat().headingLevel() == headingLevelForFormat(format);
    case Bold:
        return cursor.charFormat().fontWeight() == QFont::Bold;
    case Italic:
        return cursor.charFormat().fontItalic();
    case UnorderedList:
        return cursor.currentList()->format().style() == QTextListFormat::ListDisc;
    case OrderedList:
        return cursor.currentList()->format().style() == QTextListFormat::ListDecimal;
    case Strikethrough:
        return cursor.charFormat().fontStrikeOut();
    case Underline:
        return cursor.charFormat().fontUnderline();
    case InlineCode:
        return cursor.charFormat().fontFixedPitch();
    default:
        return false;
    }
}

bool RichFormat::hasAnyFormat(QTextCursor cursor, QList<Format> formats)
{
    for (const auto &format : formats) {
        return hasFormat(cursor, format);
    }
    return false;
}

QList<RichFormat::Format> RichFormat::formatsAtCursor(const QTextCursor &cursor)
{
    QList<Format> formats;
    if (cursor.isNull()) {
        return formats;
    }
    if (cursor.charFormat().fontWeight() == QFont::Bold) {
        formats += Bold;
    }
    if (cursor.charFormat().fontItalic()) {
        formats += Italic;
    }
    if (cursor.charFormat().fontUnderline()) {
        formats += Underline;
    }
    if (cursor.charFormat().fontStrikeOut()) {
        formats += Strikethrough;
    }
    if (cursor.charFormat().fontFixedPitch()) {
        formats += InlineCode;
    }
    if (cursor.blockFormat().headingLevel() > 0 && cursor.blockFormat().headingLevel() <= 6) {
        formats += formatForHeadingLevel(cursor.blockFormat().headingLevel());
    }
    if (cursor.currentList() && cursor.currentList()->format().style() == QTextListFormat::ListDisc) {
        formats += UnorderedList;
    }
    if (cursor.currentList() && cursor.currentList()->format().style() == QTextListFormat::ListDecimal) {
        formats += OrderedList;
    }
    return formats;
}
