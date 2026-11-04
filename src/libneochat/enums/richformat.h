// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QTextList>

class QTextBlockFormat;
class QTextCharFormat;
class QTextCursor;
class QTextDocument;

using namespace Qt::StringLiterals;

/**
 * @class RichFormat
 */
class RichFormat : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief Enum to define available formats.
     *
     * @note The Paragraph and Heading values are intentially fixed to match heading
     *       level values returned by QTextBlockFormat::headingLevel().
     *
     * @sa QTextBlockFormat::headingLevel()
     */
    enum Format {
        Paragraph = 0,
        Heading1 = 1,
        Heading2 = 2,
        Heading3 = 3,
        Heading4 = 4,
        Heading5 = 5,
        Heading6 = 6,
        Quote,
        Code,
        InlineCode,
        Bold,
        Italic,
        UnorderedList,
        OrderedList,
        Strikethrough,
        Underline,
    };
    Q_ENUM(Format);

    /**
     * @brief Enum to define the type of format.
     */
    enum FormatType {
        Text, /**< The format is applied to the text chars. */
        List, /**< The format is list style. */
        Style, /**< The format is a paragraph style. */
        Block, /**< The format changes the block type. */
    };
    Q_ENUM(FormatType);

    /**
     * @brief Translate the Format enum value to a human readable string.
     *
     * @sa Format
     */
    static QString styleString(Format format, bool inQuoteBlock = false);

    /**
     * @brief Return the FormatType for the Format.
     *
     * @sa Format, FormatType
     */
    static FormatType typeForFormat(Format format);

    /**
     * @brief Return the QTextListFormat::Style for the Format.
     *
     * @sa Format, QTextListFormat::Style
     */
    static QTextListFormat::Style listStyleForFormat(Format format);

    /**
     * @brief Return the QTextCharFormat for the Format.
     *
     * Inverting returns a format which will remove the format when merged using
     * QTextCursor::mergeCharFormat().
     *
     * @sa Format, QTextCharFormat
     */
    static QTextCharFormat charFormatForFormat(Format format, bool invert = false, const QColor &highlightColor = {});

    /**
     * @brief Return the QTextBlockFormat for the Format.
     *
     * @sa Format, QTextBlockFormat
     */
    static QTextBlockFormat blockFormatForFormat(Format format);

    /**
     * @brief Whether the given QTextCursor has the given Format.
     *
     * @sa Format, QTextCursor
     */
    static bool hasFormat(QTextCursor cursor, Format format);

    /**
     * @brief Whether the given QTextCursor has any of the given Formats.
     *
     * @sa Format, QTextCursor
     */
    static bool hasAnyFormat(QTextCursor cursor, QList<Format> formats);

    /**
     * @brief A lsit of Formats on the given QTextCursor.
     *
     * @sa Format, QTextCursor
     */
    static QList<Format> formatsAtCursor(const QTextCursor &cursor);

    /**
     * @brief Make the font properties of all text in the document explicit.
     *
     * QTextDocument::toMarkdown() resolves unset font properties against the application font,
     * so a monospace, bold or italic system font would make all text code, bold or italic.
     * Call this before converting to markdown; properties that are actually set are kept.
     */
    static void makeFontPropertiesExplicit(QTextDocument &doc);

private:
    /**
     * @brief Return the heading level for the Format.
     *
     * @sa Format
     */
    static int headingLevelForFormat(Format format);

    /**
     * @brief Return the Format for the heading level.
     *
     * @sa Format
     */
    static RichFormat::Format formatForHeadingLevel(int level);
};
