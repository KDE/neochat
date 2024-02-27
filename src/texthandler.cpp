// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "texthandler.h"

#include <QDebug>
#include <QGuiApplication>
#include <QStringLiteral>
#include <QTextBlock>
#include <QUrl>

#include <Quotient/events/roommessageevent.h>
#include <Quotient/util.h>

#include <cmark.h>

#include <Kirigami/Platform/PlatformTheme>

#include "messagecomponenttype.h"
#include "messagecontentmodel.h"
#include "models/customemojimodel.h"
#include "utils.h"

static const QStringList allowedTags = {
    QStringLiteral("font"),    QStringLiteral("del"),    QStringLiteral("h1"),         QStringLiteral("h2"),     QStringLiteral("h3"),    QStringLiteral("h4"),
    QStringLiteral("h5"),      QStringLiteral("h6"),     QStringLiteral("blockquote"), QStringLiteral("p"),      QStringLiteral("a"),     QStringLiteral("ul"),
    QStringLiteral("ol"),      QStringLiteral("sup"),    QStringLiteral("sub"),        QStringLiteral("li"),     QStringLiteral("b"),     QStringLiteral("i"),
    QStringLiteral("u"),       QStringLiteral("strong"), QStringLiteral("em"),         QStringLiteral("strike"), QStringLiteral("code"),  QStringLiteral("hr"),
    QStringLiteral("br"),      QStringLiteral("div"),    QStringLiteral("table"),      QStringLiteral("thead"),  QStringLiteral("tbody"), QStringLiteral("tr"),
    QStringLiteral("th"),      QStringLiteral("td"),     QStringLiteral("caption"),    QStringLiteral("pre"),    QStringLiteral("span"),  QStringLiteral("img"),
    QStringLiteral("details"), QStringLiteral("summary")};
static const QHash<QString, QStringList> allowedAttributes = {
    {QStringLiteral("font"), {QStringLiteral("data-mx-bg-color"), QStringLiteral("data-mx-color"), QStringLiteral("color")}},
    {QStringLiteral("span"), {QStringLiteral("data-mx-bg-color"), QStringLiteral("data-mx-color"), QStringLiteral("data-mx-spoiler")}},
    {QStringLiteral("a"), {QStringLiteral("name"), QStringLiteral("target"), QStringLiteral("href")}},
    {QStringLiteral("img"), {QStringLiteral("width"), QStringLiteral("height"), QStringLiteral("alt"), QStringLiteral("title"), QStringLiteral("src")}},
    {QStringLiteral("ol"), {QStringLiteral("start")}},
    {QStringLiteral("code"), {QStringLiteral("class")}}};
static const QStringList allowedLinkSchemes = {QStringLiteral("https"),
                                               QStringLiteral("http"),
                                               QStringLiteral("ftp"),
                                               QStringLiteral("mailto"),
                                               QStringLiteral("magnet")};
static const QStringList blockTags = {QStringLiteral("blockquote"),
                                      QStringLiteral("p"),
                                      QStringLiteral("ul"),
                                      QStringLiteral("ol"),
                                      QStringLiteral("div"),
                                      QStringLiteral("table"),
                                      QStringLiteral("pre")};

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
    return outputString;
}

QString
TextHandler::handleRecieveRichText(Qt::TextFormat inputFormat, const NeoChatRoom *room, const Quotient::RoomEvent *event, bool stripNewlines, bool isEdited)
{
    m_pos = 0;
    m_dataBuffer = m_data;

    // Strip mx-reply if present.
    m_dataBuffer.remove(TextRegex::removeRichReply);

    // For plain text, convert links, escape html and convert line brakes.
    if (inputFormat == Qt::PlainText) {
        m_dataBuffer = escapeHtml(m_dataBuffer);
        m_dataBuffer.replace(u'\n', QStringLiteral("<br>"));
    }

    // Linkify any plain text urls
    m_dataBuffer = linkifyUrls(m_dataBuffer);

    // Apply user style
    m_dataBuffer.replace(TextRegex::userPill, QStringLiteral(R"(<b>\1</b>)"));

    // Make all media URLs resolvable.
    if (room && event) {
        QRegularExpressionMatchIterator i = TextRegex::mxcImage.globalMatch(m_dataBuffer);
        while (i.hasNext()) {
            const QRegularExpressionMatch match = i.next();
            const QUrl mediaUrl = room->makeMediaUrl(event->id(), QUrl(QStringLiteral("mxc://") + match.captured(2) + u'/' + match.captured(3)));
            m_dataBuffer.replace(match.captured(0),
                                 QStringLiteral("<img ") + match.captured(1) + QStringLiteral("src=\"") + mediaUrl.toString() + u'"' + match.captured(4)
                                     + u'>');
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
            } else if ((getTagType(m_nextToken) == QStringLiteral("br") && stripNewlines)) {
                nextTokenBuffer = u' ';
            }
            nextTokenBuffer = cleanAttributes(getTagType(m_nextToken), nextTokenBuffer);
        }

        outputString.append(nextTokenBuffer);

        m_nextTokenType = nextTokenType(m_dataBuffer, m_pos, m_nextToken, m_nextTokenType);
    }

    if (isEdited) {
        if (outputString.endsWith(QStringLiteral("</p>"))) {
            outputString.insert(outputString.length() - 4, editString());
        } else if (outputString.endsWith(QStringLiteral("</pre>")) || outputString.endsWith(QStringLiteral("</blockquote>"))
                   || outputString.endsWith(QStringLiteral("</table>")) || outputString.endsWith(QStringLiteral("</ol>"))
                   || outputString.endsWith(QStringLiteral("</ul>"))) {
            outputString.append(QStringLiteral("<p>%1</p>").arg(editString()));
        } else {
            outputString.append(editString());
        }
    }

    /**
     * Replace <del> with <s>
     * Note: <s> is still not a valid tag for the message from the server. We
     * convert as that is what is needed for Qt::RichText.
     */
    outputString.replace(TextRegex::strikethrough, QStringLiteral("<s>\\1</s>"));
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
    m_dataBuffer.replace(QStringLiteral("<br />\n"), QStringLiteral("<br>"));

    if (stripNewlines) {
        m_dataBuffer.replace(QStringLiteral("<br>\n"), QStringLiteral(" "));
        m_dataBuffer.replace(QStringLiteral("<br>"), QStringLiteral(" "));
        m_dataBuffer.replace(QStringLiteral("<br />\n"), QStringLiteral(" "));
        m_dataBuffer.replace(QStringLiteral("<br />"), QStringLiteral(" "));
        m_dataBuffer.replace(u'\n', QStringLiteral(" "));
        m_dataBuffer.replace(u'\u2028', QStringLiteral(" "));
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
            if (getTagType(m_nextToken) == QStringLiteral("br") && !stripNewlines) {
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
        searchStr = QStringLiteral("</code>");
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
    } else if (currentTokenType == Type::Tag && getTagType(currentToken) == QStringLiteral("code") && !isCloseTag(currentToken)
               && string.indexOf(QStringLiteral("</code>"), currentPos) != currentPos) {
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

    int closeTagPos = string.indexOf(QStringLiteral("</%1>").arg(tagType));
    // If the close tag can't be found assume malformed html and process as single block.
    if (closeTagPos == -1) {
        return string.size();
    }

    return closeTagPos + tag.size() + 1;
}

MessageComponent TextHandler::nextBlock(const QString &string,
                                        int nextBlockPos,
                                        Qt::TextFormat inputFormat,
                                        const NeoChatRoom *room,
                                        const Quotient::RoomEvent *event,
                                        bool isEdited)
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
        attributes = getAttributes(QStringLiteral("code"), string.mid(tagEndPos + 1, string.indexOf(u'>', tagEndPos + 1) - tagEndPos));
    }

    auto content = stripBlockTags(string.first(nextBlockPos), tagType);
    setData(content);
    switch (messageComponentType) {
    case MessageComponentType::Code:
        content = unescapeHtml(content);
        break;
    default:
        content = handleRecieveRichText(inputFormat, room, event, false, isEdited);
    }
    return MessageComponent{messageComponentType, content, attributes};
}

QString TextHandler::stripBlockTags(QString string, const QString &tagType) const
{
    if (blockTags.contains(tagType) && tagType != QStringLiteral("ol") && tagType != QStringLiteral("ul") && tagType != QStringLiteral("table")) {
        string.replace(QLatin1String("<%1>").arg(tagType), QString()).replace(QLatin1String("</%1>").arg(tagType), QString());
    }

    if (string.startsWith(QStringLiteral("\n"))) {
        string.remove(0, 1);
    }
    if (string.endsWith(QStringLiteral("\n"))) {
        string.remove(string.size() - 1, string.size());
    }
    if (tagType == QStringLiteral("pre")) {
        if (string.startsWith(QStringLiteral("<code"))) {
            string.remove(0, string.indexOf(u'>') + 1);
            string.remove(string.size() - 7, string.size());
        }
        if (string.endsWith(QStringLiteral("\n"))) {
            string.remove(string.size() - 1, string.size());
        }
    }
    if (tagType == QStringLiteral("blockquote")) {
        if (string.startsWith(QStringLiteral("<p>"))) {
            string.remove(0, 3);
            string.remove(string.size() - 4, string.size());
        }
        if (!string.startsWith(u'"')) {
            string.prepend(u'"');
        }
        if (!string.endsWith(u'"')) {
            string.append(u'"');
        }
    }

    return string;
}

QString TextHandler::getTagType(const QString &tagToken) const
{
    if (tagToken.isEmpty()) {
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
        return QStringLiteral();
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
        return !linkUrl.isRelative() && linkUrl.scheme() == QStringLiteral("mxc");
    } else {
        return !linkUrl.isRelative() && allowedLinkSchemes.contains(linkUrl.scheme());
    }
}

QString TextHandler::cleanAttributes(const QString &tag, const QString &tagString)
{
    int nextAttributeIndex = tagString.indexOf(u' ', 1);

    if (nextAttributeIndex != -1) {
        QString outputString = tagString.left(nextAttributeIndex);
        QString nextAttribute;
        int nextSpaceIndex;
        nextAttributeIndex += 1;

        while (nextAttributeIndex < tagString.length()) {
            nextSpaceIndex = tagString.indexOf(TextRegex::endTagType, nextAttributeIndex);
            if (nextSpaceIndex == -1) {
                nextSpaceIndex = tagString.length();
            }
            nextAttribute = tagString.mid(nextAttributeIndex, nextSpaceIndex - nextAttributeIndex);

            if (isAllowedAttribute(tag, getAttributeType(nextAttribute))) {
                if (tag == QStringLiteral("img") && getAttributeType(nextAttribute) == QStringLiteral("src")) {
                    QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    if (isAllowedLink(attributeData, true)) {
                        outputString.append(u' ' + nextAttribute);
                    }
                } else if (tag == u'a' && getAttributeType(nextAttribute) == QStringLiteral("href")) {
                    QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    if (isAllowedLink(attributeData)) {
                        outputString.append(u' ' + nextAttribute);
                    }
                } else if (tag == QStringLiteral("code") && getAttributeType(nextAttribute) == QStringLiteral("class")) {
                    if (getAttributeData(nextAttribute).remove(u'"').startsWith(QStringLiteral("language-"))) {
                        outputString.append(u' ' + nextAttribute);
                    }
                } else {
                    outputString.append(u' ' + nextAttribute);
                }
            }
            nextAttributeIndex = nextSpaceIndex + 1;
        }

        outputString += u'>';
        return outputString;
    }

    return tagString;
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
            nextSpaceIndex = tagString.indexOf(TextRegex::endTagType, nextAttributeIndex);
            if (nextSpaceIndex == -1) {
                nextSpaceIndex = tagString.length();
            }
            nextAttribute = tagString.mid(nextAttributeIndex, nextSpaceIndex - nextAttributeIndex);

            if (isAllowedAttribute(tag, getAttributeType(nextAttribute))) {
                if (tag == QStringLiteral("img") && getAttributeType(nextAttribute) == QStringLiteral("src")) {
                    QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    if (isAllowedLink(attributeData, true)) {
                        attributes[getAttributeType(nextAttribute)] = getAttributeData(nextAttribute, true);
                    }
                } else if (tag == u'a' && getAttributeType(nextAttribute) == QStringLiteral("href")) {
                    QString attributeData = TextRegex::attributeData.match(getAttributeData(nextAttribute)).captured(1);
                    if (isAllowedLink(attributeData)) {
                        attributes[getAttributeType(nextAttribute)] = getAttributeData(nextAttribute, true);
                    }
                } else if (tag == QStringLiteral("code") && getAttributeType(nextAttribute) == QStringLiteral("class")) {
                    if (getAttributeData(nextAttribute).remove(u'"').startsWith(QStringLiteral("language-"))) {
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

QList<MessageComponent>
TextHandler::textComponents(QString string, Qt::TextFormat inputFormat, const NeoChatRoom *room, const Quotient::RoomEvent *event, bool isEdited)
{
    if (string.isEmpty()) {
        return {};
    }

    // Strip mx-reply if present.
    string.remove(TextRegex::removeRichReply);

    QList<MessageComponent> components;
    while (!string.isEmpty()) {
        const auto nextBlockPos = this->nextBlockPos(string);
        const auto nextBlock = this->nextBlock(string, nextBlockPos, inputFormat, room, event, nextBlockPos == string.size() ? isEdited : false);
        components += nextBlock;
        string.remove(0, nextBlockPos);

        if (string.startsWith(QStringLiteral("\n"))) {
            string.remove(0, 1);
        }
        string = string.trimmed();

        if (event != nullptr && room != nullptr) {
            if (auto e = eventCast<const Quotient::RoomMessageEvent>(event); e->msgtype() == Quotient::MessageEventType::Emote && components.size() == 1) {
                if (components[0].type == MessageComponentType::Text) {
                    components[0].content = emoteString(room, event) + components[0].content;
                } else {
                    components.prepend(MessageComponent{MessageComponentType::Text, emoteString(room, event), {}});
                }
            }
        }
    }

    if (isEdited && components.last().type != MessageComponentType::Text) {
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

    result.replace(QStringLiteral("<!-- raw HTML omitted -->"), QString());

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
    stringIn.replace(u'<', QStringLiteral("&lt;"));
    stringIn.replace(u'>', QStringLiteral("&gt;"));
    return stringIn;
}

QString TextHandler::unescapeHtml(QString stringIn)
{
    // For those situations where brackets in code block get double escaped
    stringIn.replace(QStringLiteral("&amp;lt;"), QStringLiteral("<"));
    stringIn.replace(QStringLiteral("&amp;gt;"), QStringLiteral(">"));
    stringIn.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    stringIn.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    stringIn.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    stringIn.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
    return stringIn;
}

QString TextHandler::linkifyUrls(QString stringIn)
{
    QRegularExpressionMatch match;
    int start = 0;
    for (int index = 0; index != -1; index = stringIn.indexOf(TextRegex::mxId, start, &match)) {
        int skip = 0;
        if (match.captured(0).size() > 0) {
            if (stringIn.left(index).count(QStringLiteral("<code>")) == stringIn.left(index).count(QStringLiteral("</code>"))) {
                auto replacement = QStringLiteral("<a href=\"https://matrix.to/#/%1\">%1</a>").arg(match.captured(2));
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
            if (stringIn.left(index).count(QStringLiteral("<code>")) == stringIn.left(index).count(QStringLiteral("</code>"))) {
                auto replacement = QStringLiteral("<a href=\"%1\">%1</a>").arg(match.captured(1));
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
            if (stringIn.left(index).count(QStringLiteral("<code>")) == stringIn.left(index).count(QStringLiteral("</code>"))) {
                auto replacement = QStringLiteral("<a href=\"mailto:%1\">%1</a>").arg(match.captured(2));
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

QString TextHandler::editString() const
{
    Kirigami::Platform::PlatformTheme *theme =
        static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));

    QString editTextColor;
    if (theme != nullptr) {
        editTextColor = theme->disabledTextColor().name();
    } else {
        editTextColor = QStringLiteral("#000000");
    }
    return QStringLiteral(" <span style=\"color:") + editTextColor + QStringLiteral("\">(edited)</span>");
}

QString TextHandler::emoteString(const NeoChatRoom *room, const Quotient::RoomEvent *event) const
{
    if (room == nullptr || event == nullptr) {
        return {};
    }

    auto e = eventCast<const Quotient::RoomMessageEvent>(event);
    auto author = room->user(e->senderId());
    return QStringLiteral("* <a href=\"https://matrix.to/#/") + e->senderId() + QStringLiteral("\" style=\"color:") + Utils::getUserColor(author->hueF()).name()
        + QStringLiteral("\">") + author->displayname(room) + QStringLiteral("</a> ");
}

QString TextHandler::convertCodeLanguageString(const QString &languageString)
{
    const int equalsPos = languageString.indexOf(u'-');
    auto data = languageString.right(languageString.length() - equalsPos - 1);

    // The standard markdown syntax uses lower case. This will get a subgroup of
    // single word languages to work.
    if (data.first(1).isLower()) {
        data[0] = data[0].toUpper();
    }

    if (data == QStringLiteral("Cpp")) {
        data = QStringLiteral("C++");
    }
    if (data == QStringLiteral("Json")) {
        data = QStringLiteral("JSON");
    }
    if (data == QStringLiteral("Html")) {
        data = QStringLiteral("HTML");
    }
    if (data == QStringLiteral("Qml")) {
        data = QStringLiteral("QML");
    }

    return data;
}

#include "moc_texthandler.cpp"
