// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "blockcache.h"

#include <QRegularExpression>

#include "chattextitemhelper.h"

using namespace Block;

inline QString formatQuote(const QString &input)
{
    QString stringOut;
    auto splitString = input.split(u"\n\n"_s, Qt::SkipEmptyParts);
    for (auto &string : splitString) {
        if (string.startsWith(u'*')) {
            string.removeFirst();
        }
        if (string.startsWith(u'\"')) {
            string.removeFirst();
        }
        if (string.endsWith(u'*')) {
            string.removeLast();
        }
        if (string.endsWith(u'\"')) {
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

inline QString trim(QString string)
{
    while (string.startsWith(u"\n"_s)) {
        string.removeFirst();
    }
    while (string.endsWith(u"\n"_s)) {
        string.removeLast();
    }
    return string;
}

QString CacheItem::toString() const
{
    auto newText = trim(content.toMarkdown(QTextDocument::MarkdownDialectGitHub));
    newText.replace(QRegularExpression(u"(?<!\n)\n(?!\n)"_s), u" "_s);
    if (type == MessageComponentType::Quote) {
        newText = formatQuote(newText);
    } else if (type == MessageComponentType::Code) {
        newText = formatCode(newText);
    }
    return newText;
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
