// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "texthandler.h"

#include <QDebug>
#include <QGuiApplication>
#include <QStringLiteral>
#include <QUrl>

#include <Quotient/events/roommessageevent.h>
#include <Quotient/util.h>
#include <qstringliteral.h>

#include <cmark.h>

#include <Kirigami/PlatformTheme>

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

    nextTokenType();

    // Strip any disallowed tags/attributes.
    QString outputString;
    while (m_pos < m_dataBuffer.length()) {
        next();

        QString nextTokenBuffer = m_nextToken;
        if (m_nextTokenType == Type::Text || m_nextTokenType == Type::TextCode) {
            nextTokenBuffer = escapeHtml(nextTokenBuffer);
        } else if (m_nextTokenType == Type::Tag) {
            if (!isAllowedTag(getTagType())) {
                nextTokenBuffer = QString();
            }
            nextTokenBuffer = cleanAttributes(getTagType(), nextTokenBuffer);
        }

        outputString.append(nextTokenBuffer);

        nextTokenType();
    }
    return outputString;
}

QString TextHandler::handleRecieveRichText(Qt::TextFormat inputFormat, const NeoChatRoom *room, const Quotient::RoomEvent *event, bool stripNewlines)
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
    nextTokenType();
    while (m_pos < m_dataBuffer.length()) {
        next();

        QString nextTokenBuffer = m_nextToken;
        if (m_nextTokenType == Type::Text || m_nextTokenType == Type::TextCode) {
            nextTokenBuffer = escapeHtml(nextTokenBuffer);
        } else if (m_nextTokenType == Type::Tag) {
            if (!isAllowedTag(getTagType())) {
                nextTokenBuffer = QString();
            } else if ((getTagType() == QStringLiteral("br") && stripNewlines)) {
                nextTokenBuffer = u' ';
            }
            nextTokenBuffer = cleanAttributes(getTagType(), nextTokenBuffer);
        }

        outputString.append(nextTokenBuffer);

        nextTokenType();
    }

    // If the message is an emote add the user pill to the front of the message.
    if (event != nullptr) {
        auto e = eventCast<const Quotient::RoomMessageEvent>(event);
        if (e->msgtype() == Quotient::MessageEventType::Emote) {
            auto author = room->user(e->senderId());
            QString emoteString = QStringLiteral("* <a href=\"https://matrix.to/#/") + e->senderId() + QStringLiteral("\" style=\"color:")
                + Utils::getUserColor(author->hueF()).name() + QStringLiteral("\">") + author->displayname(room) + QStringLiteral("</a> ");
            if (outputString.startsWith(QStringLiteral("<p>"))) {
                outputString.insert(3, emoteString);
            } else {
                outputString.prepend(emoteString);
            }
        }
    }

    if (auto e = eventCast<const Quotient::RoomMessageEvent>(event)) {
        bool isEdited =
            !e->unsignedJson().isEmpty() && e->unsignedJson().contains("m.relations") && e->unsignedJson()["m.relations"].toObject().contains("m.replace");
        if (isEdited) {
            Kirigami::PlatformTheme *theme = static_cast<Kirigami::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::PlatformTheme>(this, true));

            QString editTextColor;
            if (theme != nullptr) {
                editTextColor = theme->disabledTextColor().name();
            } else {
                editTextColor = QStringLiteral("#000000");
            }
            QString editedString = QStringLiteral(" <span style=\"color:") + editTextColor + QStringLiteral("\">(edited)</span>");
            if (outputString.endsWith(QStringLiteral("</p>"))) {
                outputString.insert(outputString.length() - 4, editedString);
            } else if (outputString.endsWith(QStringLiteral("</pre>")) || outputString.endsWith(QStringLiteral("</blockquote>"))
                       || outputString.endsWith(QStringLiteral("</table>")) || outputString.endsWith(QStringLiteral("</ol>"))
                       || outputString.endsWith(QStringLiteral("</ul>"))) {
                outputString.append("<p>" + editedString + "</p>");
            } else {
                outputString.append(editedString);
            }
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

    if (stripNewlines) {
        m_dataBuffer.replace(QStringLiteral("<br>\n"), QStringLiteral(" "));
        m_dataBuffer.replace(QStringLiteral("<br>"), QStringLiteral(" "));
        m_dataBuffer.replace(QStringLiteral("<br />\n"), QStringLiteral(" "));
        m_dataBuffer.replace(QStringLiteral("<br />"), QStringLiteral(" "));
        m_dataBuffer.replace(u'\n', QStringLiteral(" "));
        m_dataBuffer.replace(u'\u2028', " ");
    }

    // Strip all tags/attributes except code blocks which will be escaped.
    QString outputString;
    nextTokenType();
    while (m_pos < m_dataBuffer.length()) {
        next();

        QString nextTokenBuffer = m_nextToken;
        if (m_nextTokenType == Type::TextCode) {
            nextTokenBuffer = unescapeHtml(nextTokenBuffer);
        } else if (m_nextTokenType == Type::Tag) {
            nextTokenBuffer = QString();
        }

        outputString.append(nextTokenBuffer);

        nextTokenType();
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

void TextHandler::nextTokenType()
{
    if (m_pos >= m_dataBuffer.length()) {
        // This is to stop the function accessing an index outside the length of
        // m_dataBuffer during the final loop.
        m_nextTokenType = Type::End;
    } else if (m_nextTokenType == Type::Tag && getTagType() == QStringLiteral("code") && !isCloseTag()
               && m_dataBuffer.indexOf(QStringLiteral("</code>"), m_pos) != m_pos) {
        m_nextTokenType = Type::TextCode;
    } else if (m_dataBuffer[m_pos] == u'<' && m_dataBuffer[m_pos + 1] != u' ') {
        m_nextTokenType = Type::Tag;
    } else {
        m_nextTokenType = Type::Text;
    }
}

QString TextHandler::getTagType() const
{
    if (m_nextToken.isEmpty()) {
        return QString();
    }
    const int tagTypeStart = m_nextToken[1] == u'/' ? 2 : 1;
    const int tagTypeEnd = m_nextToken.indexOf(TextRegex::endTagType, tagTypeStart);
    return m_nextToken.mid(tagTypeStart, tagTypeEnd - tagTypeStart);
}

bool TextHandler::isCloseTag() const
{
    if (m_nextToken.isEmpty()) {
        return false;
    }
    return m_nextToken[1] == u'/';
}

QString TextHandler::getAttributeType(const QString &string)
{
    if (!string.contains(u'=')) {
        return string;
    }
    const int equalsPos = string.indexOf(u'=');
    return string.left(equalsPos);
}

QString TextHandler::getAttributeData(const QString &string)
{
    if (!string.contains(u'=')) {
        return QStringLiteral();
    }
    const int equalsPos = string.indexOf(u'=');
    return string.right(string.length() - equalsPos - 1);
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
        return !linkUrl.isRelative() && linkUrl.scheme() == "mxc";
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
    stringIn = stringIn.replace(TextRegex::mxId, QStringLiteral(R"(\1<a href="https://matrix.to/#/\2">\2</a>)"));
    stringIn.replace(TextRegex::plainUrl, QStringLiteral(R"(<a href="\1">\1</a>)"));
    stringIn = stringIn.replace(TextRegex::emailAddress, QStringLiteral(R"(<a href="mailto:\2">\1\2</a>)"));
    return stringIn;
}

QList<QUrl> TextHandler::getLinkPreviews()
{
    auto data = m_data.remove(TextRegex::removeRichReply);
    auto linksMatch = TextRegex::url.globalMatch(data);
    QList<QUrl> links;
    while (linksMatch.hasNext()) {
        auto link = linksMatch.next().captured();
        if (!link.contains(QStringLiteral("matrix.to"))) {
            links += QUrl(link);
        }
    }
    return links;
}

#include "moc_texthandler.cpp"
