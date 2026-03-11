// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "blockcache.h"

#include <QRegularExpression>

#include "chattextitemhelper.h"
#include "messagecomponenttype.h"

using namespace Block;

inline QString formatQuote(const QString &input)
{
    QString stringOut;
    auto splitString = input.split(u"\n\n"_s, Qt::SkipEmptyParts);
    for (auto &string : splitString) {
        if (string.startsWith(u'*')) {
            string.removeFirst();
        }
        if (string.startsWith(u'“')) {
            string.removeFirst();
        }
        if (string.endsWith(u'*')) {
            string.removeLast();
        }
        if (string.endsWith(u'”')) {
            string.removeLast();
        }
        if (!stringOut.isEmpty()) {
            stringOut += u"\n"_s;
        }
        stringOut += u"> "_s + string;
    }
    return stringOut;
}

inline QString formatCode(const QString &input)
{
    return u"```\n%1\n```"_s.arg(input).replace(u"\n\n"_s, u"\n"_s);
}

inline QString trimmedTrailing(QString string)
{
    while (string.endsWith(u' ')) {
        string.removeLast();
    }
    return string;
}

inline QString trimNewline(QString string)
{
    while (string.startsWith(u"\n"_s)) {
        string.removeFirst();
    }
    while (string.endsWith(u"\n"_s)) {
        string.removeLast();
    }
    return string;
}

bool CacheItem::richTextActive = true;

QString CacheItem::toString() const
{
    if (!richTextActive) {
        auto plainText = content.toPlainText();
        const auto markdownText = content.toMarkdown();
        QRegularExpression mentionRegex(u"\\[(.*)]\\(.*\\)"_s);
        QRegularExpressionMatch mentionMatch;
        qsizetype lastPos = 0;
        qsizetype mentionPos = markdownText.indexOf(mentionRegex, lastPos, &mentionMatch);
        while (mentionPos != -1) {
            auto mentionName = mentionMatch.captured(1);
            if (mentionName.startsWith(u"\\"_s)) {
                mentionName.remove(0, 1);
            }
            qsizetype plainPos = plainText.indexOf(mentionName);
            if (plainPos != -1) {
                plainText.replace(plainPos, mentionName.length(), mentionMatch.captured());
            }
            lastPos = mentionPos + mentionMatch.capturedLength();
            mentionPos = markdownText.indexOf(mentionRegex, lastPos, &mentionMatch);
        }
        return trimNewline(plainText);
    }
    if (type == MessageComponentType::Code) {
        return formatCode(trimNewline(content.toPlainText()));
    }

    QString textOut;
    auto doc = QTextDocument();
    auto cursor = QTextCursor(&doc);
    cursor.insertFragment(content);
    cursor.movePosition(QTextCursor::Start);
    while (!cursor.atEnd()) {
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        auto nextText = trimmedTrailing(trimNewline(cursor.selection().toMarkdown()));
        if (!cursor.currentList()) {
            nextText.replace(u'\n', u' ');
        }
        if (!textOut.isEmpty() && !nextText.isEmpty()) {
            textOut += cursor.currentList() ? u"\n"_s : u"\n\n"_s;
        }
        textOut += nextText;
        cursor.movePosition(QTextCursor::NextBlock);
    }

    textOut = trimNewline(textOut).trimmed();
    if (type == MessageComponentType::Quote) {
        textOut = formatQuote(textOut);
    }
    return textOut;
}

void Cache::fill(QList<MessageComponent> components)
{
    std::ranges::for_each(components, [this](const MessageComponent &component) {
        if (!MessageComponentType::isTextType(component.type)) {
            return;
        }
        const auto textItem = component.attributes["chatTextItemHelper"_L1].value<ChatTextItemHelper *>();
        if (!textItem) {
            return;
        }
        append(CacheItem{
            .type = component.type,
            .content = textItem->toFragment(),
        });
    });
}

QString Cache::toString() const
{
    QString text;
    std::ranges::for_each(constBegin(), constEnd(), [&text](const CacheItem &item) {
        if (!text.isEmpty()) {
            text += u"\n\n"_s;
        }
        text += item.toString();
    });
    return text;
}
