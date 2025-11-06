// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "richformat.h"

#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <qtextformat.h>

QString RichFormat::styleString(Format format)
{
    switch (format) {
    case Paragraph:
        return u"Paragraph"_s;
    case Heading1:
        return u"Heading 1"_s;
    case Heading2:
        return u"Heading 2"_s;
    case Heading3:
        return u"Heading 3"_s;
    case Heading4:
        return u"Heading 4"_s;
    case Heading5:
        return u"Heading 5"_s;
    case Heading6:
        return u"Heading 6"_s;
    case Code:
        return u"Code"_s;
    case Quote:
        return u"\"Quote\""_s;
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

QTextCharFormat RichFormat::charFormatForFormat(Format format, bool invert)
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
    default:
        return false;
    }
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
