// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

using namespace Qt::StringLiterals;

/**
 * @class TextStyle
 *
 * A class with the Style enum for available text styles.
 */
class TextStyle : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief Enum to define available styles.
     *
     * @note The Paragraph and Heading values are intentially fixed to match heading
     *       level values returned by QTextBlockFormat::headingLevel().
     *
     * @sa QTextBlockFormat::headingLevel()
     */
    enum Style {
        Paragraph = 0,
        Heading1 = 1,
        Heading2 = 2,
        Heading3 = 3,
        Heading4 = 4,
        Heading5 = 5,
        Heading6 = 6,
        Code = 7,
        Quote = 8,
    };
    Q_ENUM(Style);

    /**
     * @brief Translate the Kind enum value to a human readable string.
     *
     * @sa Kind
     */
    static QString styleString(Style style)
    {
        switch (style) {
        case Style::Paragraph:
            return u"Paragraph"_s;
        case Style::Heading1:
            return u"Heading 1"_s;
        case Style::Heading2:
            return u"Heading 2"_s;
        case Style::Heading3:
            return u"Heading 3"_s;
        case Style::Heading4:
            return u"Heading 4"_s;
        case Style::Heading5:
            return u"Heading 5"_s;
        case Style::Heading6:
            return u"Heading 6"_s;
        case Style::Code:
            return u"Code"_s;
        case Style::Quote:
            return u"\"Quote\""_s;
        default:
            return {};
        }
    };
};
