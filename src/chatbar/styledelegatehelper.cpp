// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "styledelegatehelper.h"

#include <QQuickTextDocument>
#include <QTextCursor>
#include <QTextDocument>

#include "enums/textstyle.h"

StyleDelegateHelper::StyleDelegateHelper(QObject *parent)
    : QObject(parent)
{
}

QQuickItem *StyleDelegateHelper::textItem() const
{
    return m_textItem;
}

void StyleDelegateHelper::setTextItem(QQuickItem *textItem)
{
    if (textItem == m_textItem) {
        return;
    }

    m_textItem = textItem;

    if (m_textItem) {
        if (document()) {
            formatDocument();
        }
    }

    Q_EMIT textItemChanged();
}

QTextDocument *StyleDelegateHelper::document() const
{
    if (!m_textItem) {
        return nullptr;
    }
    const auto quickDocument = qvariant_cast<QQuickTextDocument *>(m_textItem->property("textDocument"));
    return quickDocument ? quickDocument->textDocument() : nullptr;
}

void StyleDelegateHelper::formatDocument()
{
    if (!document()) {
        return;
    }

    auto cursor = QTextCursor(document());
    if (cursor.isNull()) {
        return;
    }

    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
    const auto style = static_cast<TextStyle::Style>(m_textItem->property("index").toInt());
    const auto string = TextStyle::styleString(style);

    const int headingLevel = style <= 6 ? style : 0;
    // Apparently, 5 is maximum for FontSizeAdjustment; otherwise level=1 and
    // level=2 look the same
    const int sizeAdjustment = headingLevel > 0 ? 5 - headingLevel : 0;

    QTextBlockFormat blkfmt;
    blkfmt.setHeadingLevel(headingLevel);
    cursor.mergeBlockFormat(blkfmt);

    QTextCharFormat chrfmt;
    chrfmt.setFontWeight(headingLevel > 0 ? QFont::Bold : QFont::Normal);
    chrfmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment / 2);
    if (style == TextStyle::Code) {
        chrfmt.setFontFamilies({u"monospace"_s});
    } else if (style == TextStyle::Quote) {
        chrfmt.setFontItalic(true);
    }

    cursor.mergeBlockCharFormat(chrfmt);
    cursor.insertText(string);
    cursor.endEditBlock();
}

#include "moc_styledelegatehelper.cpp"
