// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "texthandler.h"

#include <QDebug>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QStringLiteral>
#include <QTextBlock>
#include <QUrl>

#include <Quotient/events/roommessageevent.h>
#include <Quotient/util.h>

#include <cmark.h>

#include <Kirigami/Platform/PlatformTheme>

#include "messagecomponenttype.h"
#include "models/customemojimodel.h"
#include "utils.h"

using namespace Qt::StringLiterals;

static const QStringList allowedTags = {
    u"font"_s,  u"del"_s,   u"h1"_s,    u"h2"_s, u"h3"_s, u"h4"_s, u"h5"_s,      u"h6"_s,  u"blockquote"_s, u"p"_s,    u"a"_s,       u"ul"_s,     u"ol"_s,
    u"sup"_s,   u"sub"_s,   u"li"_s,    u"b"_s,  u"i"_s,  u"u"_s,  u"strong"_s,  u"em"_s,  u"strike"_s,     u"code"_s, u"hr"_s,      u"br"_s,     u"div"_s,
    u"table"_s, u"thead"_s, u"tbody"_s, u"tr"_s, u"th"_s, u"td"_s, u"caption"_s, u"pre"_s, u"span"_s,       u"img"_s,  u"details"_s, u"summary"_s};
static const QHash<QString, QStringList> allowedAttributes = {{u"font"_s, {u"data-mx-bg-color"_s, u"data-mx-color"_s, u"color"_s}},
                                                              {u"span"_s, {u"data-mx-bg-color"_s, u"data-mx-color"_s, u"data-mx-spoiler"_s}},
                                                              {u"a"_s, {u"name"_s, u"target"_s, u"href"_s}},
                                                              {u"img"_s, {u"style"_s, u"width"_s, u"height"_s, u"alt"_s, u"title"_s, u"src"_s}},
                                                              {u"ol"_s, {u"start"_s}},
                                                              {u"code"_s, {u"class"_s}}};
static const QStringList allowedLinkSchemes = {u"https"_s, u"http"_s, u"ftp"_s, u"mailto"_s, u"magnet"_s};
static const QStringList blockTags = {u"blockquote"_s, u"p"_s, u"ul"_s, u"ol"_s, u"div"_s, u"table"_s, u"pre"_s};

static const QString customEmojiStyle = u"vertical-align:bottom"_s;

QString TextHandler::data() const
{
    return m_data;
}

void TextHandler::setData(const QString &string)
{
    m_data = string;
    m_pos = 0;
}

QString TextHandler::handleSendText()
{
    m_pos = 0;
    m_dataBuffer = markdownToHTML(m_data);
    m_dataBuffer = customMarkdownToHtml(m_dataBuffer);

    m_nextTokenType = nextTokenType(m_dataBuffer, m_pos, m_nextToken, m_nextTokenType);

    // Strip any disallowed tags/attributes.
    QString outputString;
    while (m_pos < m_dataBuffer.length()) {
        next();

        QString nextTokenBuffer = m_nextToken;
        switch (m_nextTokenType) {
        case Text:
            nextTokenBuffer = escapeHtml(nextTokenBuffer);
            nextTokenBuffer = CustomEmojiModel::instance().preprocessText(nextTokenBuffer);
            break;
        case TextCode:
            nextTokenBuffer = escapeHtml(nextTokenBuffer);
            break;
        case Tag:
            if (!isAllowedTag(getTagType(m_nextToken))) {
                nextTokenBuffer = QString();
            }
            nextTokenBuffer = cleanAttributes(getTagType(m_nextToken), nextTokenBuffer);
        default:
            break;
        }

        outputString.append(nextTokenBuffer);

        m_nextTokenType = nextTokenType(m_dataBuffer, m_pos, m_nextToken, m_nextTokenType);
    }

    if (outputString.count("<p>"_L1) == 1 && outputString.count("</p>"_L1) == 1 && outputString.startsWith("<p>"_L1) && outputString.endsWith("</p>"_L1)) {
        outputString.remove("<p>"_L1);
        outputString.remove("</p>"_L1);
    }

    return outputString;
}

QString TextHandler::handleRecieveRichText(Qt::TextFormat inputFormat,
                                           const NeoChatRoom *room,
                                           const Quotient::RoomEvent *event,
                                           bool stripNewlines,
                                           bool isEdited,
                                           bool spoilerRevealed)
{
    m_pos = 0;
    m_dataBuffer = m_data;

    // Strip mx-reply if present.
    m_dataBuffer.remove(TextRegex::removeRichReply);

    // For plain text, convert links, escape html and convert line brakes.
    if (inputFormat == Qt::PlainText) {
        m_dataBuffer = escapeHtml(m_dataBuffer);
        m_dataBuffer.replace(u'\n', u"<br>"_s);
    }

    // Linkify any plain text urls
    m_dataBuffer = linkifyUrls(m_dataBuffer);

    // Apply user style
    m_dataBuffer.replace(TextRegex::userPill, uR"(<b>\1</b>)"_s);

    // Make all media URLs resolvable.
    if (room && event) {
        QRegularExpressionMatchIterator i = TextRegex::mxcImage.globalMatch(m_dataBuffer);
        while (i.hasNext()) {
            const QRegularExpressionMatch match = i.next();
            const QUrl mediaUrl = room->makeMediaUrl(event->id(), QUrl(u"mxc://"_s + match.captured(2) + u'/' + match.captured(3)));

            QStringList extraAttributes = match.captured(4).split(QChar::Space);
            const bool isEmoticon = match.captured(1).contains(u"data-mx-emoticon"_s);

            // If the image does not have an explicit width, but has a vertical-align it's most likely an emoticon.
            // We must do some pre-processing for it to show up nicely in and around text.
            if (isEmoticon) {
                // Align it properly
                extraAttributes.append(u"style=\"%1\""_s.arg(customEmojiStyle));
            }

            m_dataBuffer.replace(match.captured(0),
                                 u"<img "_s + match.captured(1) + u"src=\""_s + mediaUrl.toString() + u'"' + extraAttributes.join(QChar::Space) + u'>');
        }
    }

    // Strip any disallowed tags/attributes.
    QString outputString;
    m_nextTokenType = nextTokenType(m_dataBuffer, m_pos, m_nextToken, m_nextTokenType);
    while (m_pos < m_dataBuffer.length()) {
        next();

        QString nextTokenBuffer = m_nextToken;
        if (m_nextTokenType == Type::Text || m_nextTokenType == Type::TextCode) {
            nextTokenBuffer = escapeHtml(nextTokenBuffer);
        } else if (m_nextTokenType == Type::Tag) {
            if (!isAllowedTag(getTagType(m_nextToken))) {
                nextTokenBuffer = QString();
            } else if ((getTagType(m_nextToken) == u"br"_s && stripNewlines)) {
                nextTokenBuffer = u' ';
            }
            nextTokenBuffer = cleanAttributes(getTagType(m_nextToken), nextTokenBuffer, true, spoilerRevealed);
        }

        outputString.append(nextTokenBuffer);

        m_nextTokenType = nextTokenType(m_dataBuffer, m_pos, m_nextToken, m_nextTokenType);
    }

    if (isEdited) {
        if (outputString.endsWith(u"</p>"_s)) {
            outputString.insert(outputString.length() - 4, editString());
        } else if (outputString.endsWith(u"</pre>"_s) || outputString.endsWith(u"</blockquote>"_s) || outputString.endsWith(u"</table>"_s)
                   || outputString.endsWith(u"</ol>"_s) || outputString.endsWith(u"</ul>"_s)) {
            outputString.append(u"<p>%1</p>"_s.arg(editString()));
        } else {
            outputString.append(editString());
        }
    }

    /**
     * Replace <del> with <s>
     * Note: <s> is still not a valid tag for the message from the server. We
     * convert as that is what is needed for Qt::RichText.
     */
    outputString.replace(TextRegex::strikethrough, u"<s>\\1</s>"_s);

    if (outputString.count("<p>"_L1) == 1 && outputString.count("</p>"_L1) == 1 && outputString.startsWith("<p>"_L1) && outputString.endsWith("</p>"_L1)) {
        outputString.remove("<p>"_L1);
        outputString.remove("</p>"_L1);
    }

    return outputString;
}

QString TextHandler::handleRecievePlainText(Qt::TextFormat inputFormat, const bool &stripNewlines)
{
    m_pos = 0;
    m_dataBuffer = m_data;

    // Strip mx-reply if present.
    m_dataBuffer.remove(TextRegex::removeRichReply);

    // Escaping then unescaping allows < and > to be maintained in a plain text string
    // otherwise markdownToHTML will strip what it thinks is a bad html tag entirely.
    if (inputFormat == Qt::PlainText) {
        m_dataBuffer = escapeHtml(m_dataBuffer);
    }

    /**
     * This seems counterproductive but by converting any markup which could
     * arrive (e.g. in a caption body) it can then be stripped by the same code.
     */
    m_dataBuffer = markdownToHTML(m_dataBuffer);
    // This is how \n is converted and for plain text we need it to just be <br>
    // to prevent extra newlines being inserted.
    m_dataBuffer.replace(u"<br />\n"_s, u"<br>"_s);

    if (stripNewlines) {
        m_dataBuffer.replace(u"<br>\n"_s, u" "_s);
        m_dataBuffer.replace(u"<br>"_s, u" "_s);
        m_dataBuffer.replace(u"<br />\n"_s, u" "_s);
        m_dataBuffer.replace(u"<br />"_s, u" "_s);
        m_dataBuffer.replace(u'\n', u" "_s);
        m_dataBuffer.replace(u'\u2028', u" "_s);
    }

    // Strip all tags/attributes except code blocks which will be escaped.
    QString outputString;
    m_nextTokenType = nextTokenType(m_dataBuffer, m_pos, m_nextToken, m_nextTokenType);
    while (m_pos < m_dataBuffer.length()) {
        next();

        QString nextTokenBuffer = m_nextToken;
        if (m_nextTokenType == Type::TextCode) {
            nextTokenBuffer = unescapeHtml(nextTokenBuffer);
        } else if (m_nextTokenType == Type::Tag) {
            if (getTagType(m_nextToken) == u"br"_s && !stripNewlines) {
                nextTokenBuffer = u'\n';
            } else {
                nextTokenBuffer = QString();
            }
        }

        outputString.append(nextTokenBuffer);

        m_nextTokenType = nextTokenType(m_dataBuffer, m_pos, m_nextToken, m_nextTokenType);
    }

    // Escaping then unescaping allows < and > to be maintained in a plain text string
    // otherwise markdownToHTML will strip what it thinks is a bad html tag entirely.
    outputString = unescapeHtml(outputString);

    outputString = outputString.trimmed();
    return outputString;
}

void TextHandler::next()
{
    QString searchStr;
    if (m_nextTokenType == Type::Tag) {
        searchStr = u'>';
    } else if (m_nextTokenType == Type::TextCode) {
        // Anything between code tags is assumed to be plain text
        searchStr = u"</code>"_s;
    } else {
        searchStr = u'<';
    }

    int tokenEnd = m_dataBuffer.indexOf(searchStr, m_pos + 1);
    if (tokenEnd == -1) {
        tokenEnd = m_dataBuffer.length();
    }

    m_nextToken = m_dataBuffer.mid(m_pos, tokenEnd - m_pos + (m_nextTokenType == Type::Tag ? 1 : 0));
    m_pos = tokenEnd + (m_nextTokenType == Type::Tag ? 1 : 0);
}

TextHandler::Type TextHandler::nextTokenType(const QString &string, int currentPos, const QString &currentToken, Type currentTokenType) const
{
    if (currentPos >= string.length()) {
        // This is to stop the function accessing an index outside the length of
        // string during the final loop.
        return Type::End;
    } else if (currentTokenType == Type::Tag && getTagType(currentToken) == u"code"_s && !isCloseTag(currentToken)
               && string.indexOf(u"</code>"_s, currentPos) != currentPos) {
        return Type::TextCode;
    } else if (string[currentPos] == u'<' && string[currentPos + 1] != u' ') {
        return Type::Tag;
    } else {
        return Type::Text;
    }
}

int TextHandler::nextBlockPos(const QString &string)
{
    if (string.isEmpty()) {
        return -1;
    }

    const auto nextTokenType = this->nextTokenType(string, 0, {}, Text);
    // If there is no tag at the start we need to handle potentially having some
    // text with no <p> tag.
    if (nextTokenType == Text) {
        int pos = 0;
        while (pos < string.size()) {
            pos = string.indexOf(u'<', pos);
            if (pos == -1) {
                pos = string.size();
            } else {
                const auto tagType = getTagType(string.mid(pos, string.indexOf(u'>', pos) - pos));
                if (blockTags.contains(tagType)) {
                    return pos;
                }
            }
            pos++;
        }
        return string.size();
    }

    int tagEndPos = string.indexOf(u'>');
    QString tag = string.first(tagEndPos + 1);
    QString tagType = getTagType(tag);
    // If the start tag is not a block tag there can be only 1 block.
    if (!blockTags.contains(tagType)) {
        return string.size();
    }

    const auto closeTag = u"</%1>"_s.arg(tagType);
    int closeTagPos = string.indexOf(closeTag);
    // If the close tag can't be found assume malformed html and process as single block.
    if (closeTagPos == -1) {
        return string.size();
    }

    return closeTagPos + closeTag.size();
}

MessageComponent TextHandler::nextBlock(const QString &string,
                                        int nextBlockPos,
                                        Qt::TextFormat inputFormat,
                                        const NeoChatRoom *room,
                                        const Quotient::RoomEvent *event,
                                        bool isEdited,
                                        bool spoilerRevealed)
{
    if (string.isEmpty()) {
        return {};
    }

    int tagEndPos = string.indexOf(u'>');
    QString tag = string.first(tagEndPos + 1);
    QString tagType = getTagType(tag);
    const auto messageComponentType = MessageComponentType::typeForTag(tagType);
    QVariantMap attributes;
    if (messageComponentType == MessageComponentType::Code) {
        attributes = getAttributes(u"code"_s, string.mid(tagEndPos + 1, string.indexOf(u'>', tagEndPos + 1) - tagEndPos));
    }

    auto content = stripBlockTags(string.first(nextBlockPos), tagType);
    setData(content);
    switch (messageComponentType) {
    case MessageComponentType::Code:
        content = unescapeHtml(content);
        break;
    default:
        content = handleRecieveRichText(inputFormat, room, event, false, isEdited, spoilerRevealed);
    }

    if (content.contains(u"data-mx-spoiler"_s)) {
        attributes[u"hasSpoiler"_s] = true;
    }
    return MessageComponent{messageComponentType, content, attributes};
}

QString TextHandler::stripBlockTags(QString string, const QString &tagType) const
{
    if (blockTags.contains(tagType) && tagType != u"ol"_s && tagType != u"ul"_s && tagType != u"table"_s && string.startsWith(u"<%1"_s.arg(tagType))) {
        string.remove(0, string.indexOf(u'>') + 1).remove(string.indexOf(u"</%1>"_s.arg(tagType)), string.size());
    }

    if (string.startsWith(u"\n"_s)) {
        string.remove(0, 1);
    }
    if (string.endsWith(u"\n"_s)) {
        string.remove(string.size() - 1, string.size());
    }
    if (tagType == u"pre"_s) {
        if (string.startsWith(u"<code"_s)) {
            string.remove(0, string.indexOf(u'>') + 1);
            string.remove(string.indexOf(u"</code>"_s), string.size());
        }
        if (string.endsWith(u"\n"_s)) {
            string.remove(string.size() - 1, string.size());
        }
    }
    if (tagType == u"blockquote"_s) {
        int startQuotationIndex = 0;
        int endQuotationIndex = string.size();

        // We have to insert the quotation marks inside of the existing
        // paragraphs, otherwise we add unnecessary line breaks.
        if (string.startsWith(u"<p>"_s)) {
            startQuotationIndex = string.indexOf(u">") + 1;
            endQuotationIndex = string.lastIndexOf(u"</p>") + 1;
        }

        // This is not a normal quotation mark but U+201C
        string.insert(startQuotationIndex, u"\""_s);
        // This is U+201D
        string.insert(endQuotationIndex, u"\""_s);
    }

    return string;
}

QString TextHandler::getTagType(const QString &tagToken) const
{
    if (tagToken.isEmpty() || tagToken.length() < 2) {
        return QString();
    }
    const int tagTypeStart = tagToken[1] == u'/' ? 2 : 1;
    const int tagTypeEnd = tagToken.indexOf(TextRegex::endTagType, tagTypeStart);
    return tagToken.mid(tagTypeStart, tagTypeEnd - tagTypeStart);
}

bool TextHandler::isCloseTag(const QString &tagToken) const
{
    if (tagToken.isEmpty()) {
        return false;
    }
    return tagToken[1] == u'/';
}

QString TextHandler::getAttributeType(const QString &string)
{
    if (!string.contains(u'=')) {
        return string;
    }
    const int equalsPos = string.indexOf(u'=');
    return string.left(equalsPos);
}

QString TextHandler::getAttributeData(const QString &string, bool stripQuotes)
{
    if (!string.contains(u'=')) {
        return QString();
    }
    const int equalsPos = string.indexOf(u'=');
    auto data = string.right(string.length() - equalsPos - 1);
    if (stripQuotes) {
        data = TextRegex::attributeData.match(data).captured(1);
    }
    return data;
}

bool TextHandler::isAllowedTag(const QString &type)
{
    return allowedTags.contains(type);
}

bool TextHandler::isAllowedAttribute(const QString &tag, const QString &attribute)
{
    return allowedAttributes[tag].contains(attribute);
}

bool TextHandler::isAllowedLink(const QString &link, bool isImg)
{
    const QUrl linkUrl = QUrl(link);

    if (isImg) {
        return !linkUrl.isRelative() && linkUrl.scheme() == u"mxc"_s;
    } else {
        return !linkUrl.isRelative() && allowedLinkSchemes.contains(linkUrl.scheme());
    }
}

QString TextHandler::cleanAttributes(const QString &tag, const QString &tagString, bool addStyle, bool spoilerRevealed)
{
    if (!tagString.contains(u'<') || !tagString.contains(u'>')) {
        return tagString;
    }
    int nextAttributeIndex = tagString.indexOf(u' ', 1);

    if (nextAttributeIndex != -1) {
        QString outputString = tagString.left(nextAttributeIndex);
        QString nextAttribute;
        int nextSpaceIndex;
        nextAttributeIndex += 1;

        while (nextAttributeIndex < tagString.length()) {
            nextSpaceIndex = tagString.indexOf(TextRegex::endAttributeType, nextAttributeIndex);
            if (nextSpaceIndex == -1) {
                nextSpaceIndex = tagString.length();
            }
            nextAttribute = tagString.mid(nextAttributeIndex, nextSpaceIndex - nextAttributeIndex);

            if (isAllowedAttribute(tag, getAttributeType(nextAttribute))) {
                QString style;
                if (tag == u"img"_s && getAttributeType(nextAttribute) == u"src"_s) {
                    QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    if (isAllowedLink(attributeData, true)) {
                        outputString.append(u' ' + nextAttribute);
                    }
                } else if (tag == u'a' && getAttributeType(nextAttribute) == u"href"_s) {
                    QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    if (isAllowedLink(attributeData)) {
                        outputString.append(u' ' + nextAttribute);
                    }
                } else if (tag == u"code"_s && getAttributeType(nextAttribute) == u"class"_s) {
                    if (getAttributeData(nextAttribute).remove(u'"').startsWith(u"language-"_s)) {
                        outputString.append(u' ' + nextAttribute);
                    }
                } else if (tag == u"img"_s && getAttributeType(nextAttribute) == u"style"_s) {
                    const QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    // Ignore every other style attribute except for our own, which we use to align custom emoticons
                    if (attributeData == customEmojiStyle) {
                        outputString.append(u' ' + nextAttribute);
                    }
                } else if (getAttributeType(nextAttribute) == u"data-mx-color"_s) {
                    const QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    style.append(u"color: "_s + attributeData + u';');
                } else if (getAttributeType(nextAttribute) == u"data-mx-bg-color"_s) {
                    const QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    style.append(u"background-color: "_s + attributeData + u';');
                } else {
                    outputString.append(u' ' + nextAttribute);
                }

                if (!style.isEmpty()) {
                    outputString.append(u" style=\""_s + style + u'"');
                }
            }
            nextAttributeIndex = nextSpaceIndex + 1;
        }

        return addStyle ? this->addStyle(tag, outputString, spoilerRevealed) : outputString + u'>';
    }

    return addStyle ? this->addStyle(tag, tagString) : tagString;
}

QString TextHandler::addStyle(const QString &tag, QString cleanTagString, bool spoilerRevealed)
{
    if (cleanTagString.endsWith(u'>')) {
        cleanTagString.removeLast();
    }

    if (!cleanTagString.startsWith(u"</"_s)) {
        if (tag == u"a"_s) {
            cleanTagString += u" style=\"text-decoration: none;\""_s;
        } else if (tag == u"table"_s) {
            cleanTagString += u" style=\"width: 100%; border-collapse: collapse; border: 1px; border-style: solid;\""_s;
        } else if (tag == u"th"_s || tag == u"td"_s) {
            cleanTagString += u" style=\"border: 1px solid black; padding: 3px;\""_s;
        } else if (tag == u"span"_s && cleanTagString.contains(u"data-mx-spoiler"_s)) {
            Kirigami::Platform::PlatformTheme *theme =
                static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
            cleanTagString += u" style=\"color: %1; background: %2;\""_s.arg(spoilerRevealed ? theme->highlightedTextColor().name() : u"transparent"_s,
                                                                             theme->alternateBackgroundColor().name());
        }
    }
    return cleanTagString + u'>';
}

QVariantMap TextHandler::getAttributes(const QString &tag, const QString &tagString)
{
    QVariantMap attributes;
    int nextAttributeIndex = tagString.indexOf(u' ', 1);

    if (nextAttributeIndex != -1) {
        QString nextAttribute;
        int nextSpaceIndex;
        nextAttributeIndex += 1;

        while (nextAttributeIndex < tagString.length()) {
            nextSpaceIndex = tagString.indexOf(TextRegex::endAttributeType, nextAttributeIndex);
            if (nextSpaceIndex == -1) {
                nextSpaceIndex = tagString.length();
            }
            nextAttribute = tagString.mid(nextAttributeIndex, nextSpaceIndex - nextAttributeIndex);

            if (isAllowedAttribute(tag, getAttributeType(nextAttribute))) {
                if (tag == u"img"_s && getAttributeType(nextAttribute) == u"src"_s) {
                    QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    if (isAllowedLink(attributeData, true)) {
                        attributes[getAttributeType(nextAttribute)] = getAttributeData(nextAttribute, true);
                    }
                } else if (tag == u'a' && getAttributeType(nextAttribute) == u"href"_s) {
                    QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    if (isAllowedLink(attributeData)) {
                        attributes[getAttributeType(nextAttribute)] = getAttributeData(nextAttribute, true);
                    }
                } else if (tag == u"code"_s && getAttributeType(nextAttribute) == u"class"_s) {
                    if (getAttributeData(nextAttribute).remove(u'"').startsWith(u"language-"_s)) {
                        attributes[getAttributeType(nextAttribute)] = convertCodeLanguageString(getAttributeData(nextAttribute, true));
                    }
                } else {
                    attributes[getAttributeType(nextAttribute)] = getAttributeData(nextAttribute, true);
                }
            }
            nextAttributeIndex = nextSpaceIndex + 1;
        }
    }
    return attributes;
}

QList<MessageComponent> TextHandler::textComponents(QString string,
                                                    Qt::TextFormat inputFormat,
                                                    const NeoChatRoom *room,
                                                    const Quotient::RoomEvent *event,
                                                    bool isEdited,
                                                    bool spoilerRevealed)
{
    if (string.trimmed().isEmpty()) {
        return {MessageComponent{MessageComponentType::Text, i18n("<i>This event does not have any content.</i>"), {}}};
    }

    // Strip mx-reply if present.
    string.remove(TextRegex::removeRichReply);

    QList<MessageComponent> components;
    while (!string.isEmpty()) {
        const auto nextBlockPos = this->nextBlockPos(string);
        const auto nextBlock =
            this->nextBlock(string, nextBlockPos, inputFormat, room, event, nextBlockPos == string.size() ? isEdited : false, spoilerRevealed);
        components += nextBlock;
        string.remove(0, nextBlockPos);

        if (string.startsWith(u"\n"_s)) {
            string.remove(0, 1);
        }
        string = string.trimmed();

        if (event != nullptr && room != nullptr) {
            if (auto e = eventCast<const Quotient::RoomMessageEvent>(event); e && e->msgtype() == Quotient::MessageEventType::Emote && components.size() == 1) {
                if (components[0].type == MessageComponentType::Text) {
                    components[0].display = emoteString(room, event) + components[0].display;
                } else {
                    components.prepend(MessageComponent{MessageComponentType::Text, emoteString(room, event), {}});
                }
            }
        }
    }

    if (isEdited && components.last().type != MessageComponentType::Text && components.last().type != MessageComponentType::Quote) {
        components += MessageComponent{MessageComponentType::Text, editString(), {}};
    }

    return components;
}

QString TextHandler::markdownToHTML(const QString &markdown)
{
    const auto str = markdown.toUtf8();
    char *tmp_buf = cmark_markdown_to_html(str.constData(), str.size(), CMARK_OPT_HARDBREAKS | CMARK_OPT_UNSAFE);

    const std::string html(tmp_buf);

    free(tmp_buf);

    auto result = QString::fromStdString(html).trimmed();

    result.replace(u"<!-- raw HTML omitted -->"_s, QString());

    return result;
}

/**
 * TODO: make this more intelligent currently other characters are not escaped
 * especially & as this can conflict with the cmark markdown to html conversion
 * which already escapes characters in code blocks. The < > still need to be handled
 * when the user manually types in the html.
 */
QString TextHandler::escapeHtml(QString stringIn)
{
    stringIn.replace(u'<', u"&lt;"_s);
    stringIn.replace(u'>', u"&gt;"_s);
    return stringIn;
}

QString TextHandler::unescapeHtml(QString stringIn)
{
    // For those situations where brackets in code block get double escaped
    stringIn.replace(u"&amp;lt;"_s, u"<"_s);
    stringIn.replace(u"&amp;gt;"_s, u">"_s);
    stringIn.replace(u"&lt;"_s, u"<"_s);
    stringIn.replace(u"&gt;"_s, u">"_s);
    stringIn.replace(u"&amp;"_s, u"&"_s);
    stringIn.replace(u"&quot;"_s, u"\""_s);
    stringIn.replace(u"&#x27;"_s, u"'"_s);
    return stringIn;
}

QString TextHandler::linkifyUrls(QString stringIn)
{
    QRegularExpressionMatch match;
    int start = 0;
    for (int index = 0; index != -1; index = stringIn.indexOf(TextRegex::mxId, start, &match)) {
        int skip = 0;
        if (match.captured(0).size() > 0) {
            if (stringIn.left(index).count(u"<code>"_s) == stringIn.left(index).count(u"</code>"_s)) {
                auto replacement = u"<a href=\"https://matrix.to/#/%1\">%1</a>"_s.arg(match.captured(1));
                stringIn = stringIn.replace(index, match.captured(0).size(), replacement);
            } else {
                skip = match.captured().length();
            }
        }
        start = index + skip;
        match = {};
    }
    start = 0;
    match = {};
    for (int index = 0; index != -1; index = stringIn.indexOf(TextRegex::plainUrl, start, &match)) {
        int skip = 0;
        if (match.captured(0).size() > 0) {
            if (stringIn.left(index).count(u"<code>"_s) == stringIn.left(index).count(u"</code>"_s)) {
                auto replacement = u"<a href=\"%1\">%1</a>"_s.arg(match.captured(1));
                stringIn = stringIn.replace(index, match.captured(0).size(), replacement);
                skip = replacement.length();
            } else {
                skip = match.captured().length();
            }
        }
        start = index + skip;
        match = {};
    }
    start = 0;
    match = {};
    for (int index = 0; index != -1; index = stringIn.indexOf(TextRegex::emailAddress, start, &match)) {
        int skip = 0;
        if (match.captured(0).size() > 0) {
            if (stringIn.left(index).count(u"<code>"_s) == stringIn.left(index).count(u"</code>"_s)) {
                auto replacement = u"<a href=\"mailto:%1\">%1</a>"_s.arg(match.captured(2));
                stringIn = stringIn.replace(index, match.captured(0).size(), replacement);
                skip = replacement.length();
            } else {
                skip = match.captured().length();
            }
        }
        start = index + skip;
        match = {};
    }

    return stringIn;
}

QString TextHandler::customMarkdownToHtml(const QString &stringIn)
{
    QString buffer = stringIn;

    const auto processSyntax = [&buffer](const QString &syntax, const QString &beginTag, const QString &endTag) {
        qsizetype beginCodeBlockTag = buffer.indexOf(u"<code>"_s);
        qsizetype endCodeBlockTag = buffer.indexOf(u"</code>"_s, beginCodeBlockTag + 1);

        // Index to search from
        qsizetype lastPos = 0;
        while (true) {
            const qsizetype pos = buffer.indexOf(syntax, lastPos);
            if (pos == -1) {
                break;
            }

            // If we're inside a code block, ignore and move the search past this code block
            const bool validCodeBlock = beginCodeBlockTag != -1 && endCodeBlockTag != -1;
            if (validCodeBlock && pos > beginCodeBlockTag && pos < endCodeBlockTag) {
                lastPos = endCodeBlockTag + 7;

                // since we moved past this code block, make sure to update the indices for the next one
                beginCodeBlockTag = buffer.indexOf(u"<code>"_s, lastPos + 1);
                endCodeBlockTag = buffer.indexOf(u"</code>"_s, beginCodeBlockTag + 1);

                continue;
            }

            qsizetype nextPos = buffer.indexOf(syntax, pos + 1);
            if (nextPos == -1) {
                break;
            }

            // Replace the beginning syntax
            buffer.replace(pos, syntax.length(), beginTag);

            // Update positions and re-search since the underlying text buffer changed
            nextPos = buffer.indexOf(syntax, pos + 1);

            // Now replace the end syntax
            buffer.replace(nextPos, syntax.length(), endTag);

            // If we have begun checking spoilers past our current code block, make sure we're in the next one (if it exists)
            if (nextPos > endCodeBlockTag) {
                beginCodeBlockTag = buffer.indexOf(u"<code>"_s, nextPos + 1);
                endCodeBlockTag = buffer.indexOf(u"</code>"_s, beginCodeBlockTag + 1);
            }

            // Move the search pointer past this point.
            // Not technically needed in most cases since we replaced the original tag, but needed for code blocks
            // which still have the characters.
            lastPos = nextPos + syntax.length();
        }
    };

    // spoilers
    processSyntax(u"||"_s, u"<span data-mx-spoiler>"_s, u"</span>"_s);

    // strikethrough
    processSyntax(u"~~"_s, u"<del>"_s, u"</del>"_s);

    // underline
    processSyntax(u"_"_s, u"<u>"_s, u"</u>"_s);

    return buffer;
}

QString TextHandler::editString() const
{
    Kirigami::Platform::PlatformTheme *theme =
        static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));

    QString editTextColor;
    if (theme != nullptr) {
        editTextColor = theme->disabledTextColor().name();
    } else {
        editTextColor = u"#000000"_s;
    }
    return u" <span style=\"color:"_s + editTextColor + u"\">(edited)</span>"_s;
}

QString TextHandler::emoteString(const NeoChatRoom *room, const Quotient::RoomEvent *event) const
{
    if (room == nullptr || event == nullptr) {
        return {};
    }

    auto e = eventCast<const Quotient::RoomMessageEvent>(event);
    auto author = room->member(e->senderId());
    return u"* <a href=\"https://matrix.to/#/"_s + e->senderId() + u"\" style=\"color:"_s + author.color().name() + u"\">"_s + author.htmlSafeDisplayName()
        + u"</a> "_s;
}

QString TextHandler::convertCodeLanguageString(const QString &languageString)
{
    const int equalsPos = languageString.indexOf(u'-');
    return languageString.right(languageString.length() - equalsPos - 1);
}

QString TextHandler::updateSpoilerText(QObject *object, QString string, bool spoilerRevealed)
{
    auto it = QRegularExpression(u"<span[^>]*data-mx-spoiler[^>]*style=\"color: (.*?); background: (.*?);\">"_s).globalMatch(string);
    Kirigami::Platform::PlatformTheme *theme =
        static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(object, true));
    int offset = 0;
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const auto newColor = spoilerRevealed ? theme->textColor().name() : u"transparent"_s;
        string.replace(match.capturedStart(2) + offset, match.capturedLength(2), theme->alternateBackgroundColor().name());
        string.replace(match.capturedStart(1) + offset, match.capturedLength(1), newColor);
        offset = newColor.length() - match.capturedLength(1);
    }
    return string;
}

#include "moc_texthandler.cpp"
