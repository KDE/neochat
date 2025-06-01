// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "texthandler.h"

#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

#include <Kirigami/Platform/PlatformTheme>

#include "enums/messagecomponenttype.h"
#include "models/customemojimodel.h"
#include "neochatconnection.h"

#include "testutils.h"

using namespace Quotient;

class TextHandlerTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();

    void allowedAttributes();
    void stripDisallowedTags();
    void stripDisallowedAttributes();
    void emptyCodeTags();

    void sendSimpleStringCase();
    void sendSingleParaMarkup();
    void sendMultipleSectionMarkup();
    void sendBadLinks();
    void sendEscapeCode();
    void sendCodeClass();
    void sendCustomEmoji();
    void sendCustomEmojiCode_data();
    void sendCustomEmojiCode();
    void sendCustomTags_data();
    void sendCustomTags();

    void receiveSpacelessSelfClosingTag();
    void receiveStripReply();
    void receivePlainTextIn();

    void receiveRichInPlainOut_data();
    void receiveRichInPlainOut();
    void receivePlainStripHtml();
    void receivePlainStripMarkup();
    void receiveStripNewlines();

    void receiveRichUserPill();
    void receiveRichStrikethrough();
    void receiveRichtextIn();
    void receiveRichMxcUrl();
    void receiveRichPlainUrl_data();
    void receiveRichPlainUrl();
    void receiveRichEdited_data();
    void receiveRichEdited();
    void receiveLineSeparator();
    void receiveRichCodeUrl();
    void receiveRichColor();

    void componentOutput_data();
    void componentOutput();
};

void TextHandlerTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
    connection->setAccountData(u"im.ponies.user_emotes"_s,
                               QJsonObject{{"images"_L1,
                                            QJsonObject{{"test"_L1,
                                                         QJsonObject{{"body"_L1, "Test custom emoji"_L1},
                                                                     {"url"_L1, "mxc://example.org/test"_L1},
                                                                     {"usage"_L1, QJsonArray{"emoticon"_L1}}}}}}});
    CustomEmojiModel::instance().setConnection(static_cast<NeoChatConnection *>(connection));

    room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-texthandler-sync.json"_s);
}

void TextHandlerTest::allowedAttributes()
{
    const QString testInputString1 = u"<span data-mx-spoiler><font color=#FFFFFF>Test</font><span>"_s;
    const QString testOutputString1 = u"<span data-mx-spoiler><font color=#FFFFFF>Test</font><span>"_s;
    // Handle urls where the href has either single (') or double (") quotes.
    const QString testInputString2 = u"<a href=\"https://kde.org\">link</a><a href='https://kde.org'>link</a>"_s;
    const QString testOutputString2 = u"<a href=\"https://kde.org\">link</a><a href='https://kde.org'>link</a>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString1);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString1);
    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString1);

    testTextHandler.setData(testInputString2);
    QCOMPARE(testTextHandler.handleSendText(), testOutputString2);
    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString2);
}

void TextHandlerTest::stripDisallowedTags()
{
    const QString testInputString = u"<p>Allowed</p> <span>Allowed</span> <body>Disallowed</body>"_s;
    const QString testOutputString = u"<p>Allowed</p> <span>Allowed</span> Disallowed"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::stripDisallowedAttributes()
{
    const QString testInputString = u"<p style=\"font-size:50px;\" color=#FFFFFF>Test</p>"_s;
    const QString testOutputString = u"Test"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

/**
 * Make sure that empty code tags are handled.
 * (this was a bug during development hence the test)
 */
void TextHandlerTest::emptyCodeTags()
{
    const QString testInputString = u"<pre><code></code></pre>"_s;
    const QString testOutputString = u"<pre><code></code></pre>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::sendSimpleStringCase()
{
    const QString testInputString = u"This data should just be left alone."_s;
    const QString testOutputString = u"This data should just be left alone."_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendSingleParaMarkup()
{
    const QString testInputString =
        u"Text para with **bold**, *italic*, [link](https://kde.org), ![image](mxc://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e), `inline code`."_s;
    const QString testOutputString =
        u"Text para with <strong>bold</strong>, <em>italic</em>, <a href=\"https://kde.org\">link</a>, <img src=\"mxc://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e\" alt=\"image\">, <code>inline code</code>."_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendMultipleSectionMarkup()
{
    const QString testInputString =
        u"Text para\n> blockquote\n* List 1\n* List 2\n1. one\n2. two\n# Heading 1\n## Heading 2\nhorizontal rule\n\n---\n```\ncodeblock\n```"_s;
    const QString testOutputString =
        u"<p>Text para</p>\n<blockquote>\n<p>blockquote</p>\n</blockquote>\n<ul>\n<li>List 1</li>\n<li>List 2</li>\n</ul>\n<ol>\n<li>one</li>\n<li>two</li>\n</ol>\n<h1>Heading 1</h1>\n<h2>Heading 2</h2>\n<p>horizontal rule</p>\n<hr>\n<pre><code>codeblock\n</code></pre>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendBadLinks()
{
    const QString testInputString = u"[link](kde.org), ![image](https://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e)"_s;
    const QString testOutputString = u"<a>link</a>, <img alt=\"image\">"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

/**
 * All text between code tags is treated as plain so it should get escaped.
 */
void TextHandlerTest::sendEscapeCode()
{
    const QString testInputString = u"```\n<p>Test <span style=\"font-size:50px;\">some</span> code</p>\n```"_s;
    const QString testOutputString =
        u"<pre><code>&lt;p&gt;Test &lt;span style=&quot;font-size:50px;&quot;&gt;some&lt;/span&gt; code&lt;/p&gt;\n</code></pre>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendCodeClass()
{
    const QString testInputString = u"```html\nsome code\n```\n<pre><code class=\"code-underline\">some more code</code></pre>"_s;
    const QString testOutputString = u"<pre><code class=\"language-html\">some code\n</code></pre>\n<pre><code>some more code</code></pre>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendCustomEmoji()
{
    const QString testInputString = u":test:"_s;
    const QString testOutputString =
        u"<img data-mx-emoticon=\"\" src=\"mxc://example.org/test\" alt=\":test:\" title=\":test:\" height=\"32\" vertical-align=\"middle\" />"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendCustomEmojiCode_data()
{
    QTest::addColumn<QString>("testInputString");
    QTest::addColumn<QString>("testOutputString");

    QTest::newRow("inline") << u"`:test:`"_s << u"<code>:test:</code>"_s;
    QTest::newRow("block") << u"```\n:test:\n```"_s << u"<pre><code>:test:\n</code></pre>"_s;
}

// Custom emojis in code blocks should be left alone.
void TextHandlerTest::sendCustomEmojiCode()
{
    QFETCH(QString, testInputString);
    QFETCH(QString, testOutputString);

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendCustomTags_data()
{
    QTest::addColumn<QString>("testInputString");
    QTest::addColumn<QString>("testOutputString");

    // spoiler
    QTest::newRow("incomplete spoiler") << u"||test"_s << u"||test"_s;
    QTest::newRow("complete spoiler") << u"||test||"_s << u"<span data-mx-spoiler>test</span>"_s;
    QTest::newRow("multiple spoiler") << u"||apple||banana||pear||"_s << u"<span data-mx-spoiler>apple</span>banana<span data-mx-spoiler>pear</span>"_s;
    QTest::newRow("inside code block spoiler") << u"```||apple||```"_s << u"<code>||apple||</code>"_s;
    QTest::newRow("outside code block spoiler") << u"||apple|| ```||banana||``` ||pear||"_s
                                                << u"<span data-mx-spoiler>apple</span> <code>||banana||</code> <span data-mx-spoiler>pear</span>"_s;
    QTest::newRow("complex spoiler") << u"Between `formFactor == Horizontal||Vertical` and `location == top||left||bottom||right`"_s
                                     << u"Between <code>formFactor == Horizontal||Vertical</code> and <code>location == top||left||bottom||right</code>"_s;

    // strikethrough
    QTest::newRow("incomplete strikethrough") << u"~~test"_s << u"~~test"_s;
    QTest::newRow("complete strikethrough") << u"~~test~~"_s << u"<del>test</del>"_s;
    QTest::newRow("inside code block strikethrough") << u"```~~apple~~```"_s << u"<code>~~apple~~</code>"_s;
    QTest::newRow("outside code block strikethrough") << u"~~apple~~ ```~~banana~~``` ~~pear~~"_s
                                                      << u"<del>apple</del> <code>~~banana~~</code> <del>pear</del>"_s;
}

void TextHandlerTest::sendCustomTags()
{
    QFETCH(QString, testInputString);
    QFETCH(QString, testOutputString);

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::receiveSpacelessSelfClosingTag()
{
    const QString testInputString = u"Test...<br/>...ing"_s;
    const QString testRichOutputString = u"Test...<br/>...ing"_s;
    const QString testPlainOutputString = u"Test...\n...ing"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testRichOutputString);
    QCOMPARE(testTextHandler.handleRecievePlainText(Qt::RichText), testPlainOutputString);
}

void TextHandlerTest::receiveStripReply()
{
    const QString testInputString =
        u"<mx-reply><blockquote><a href=\"https://matrix.to/#/!somewhere:example.org/$event:example.org\">In reply to</a><a href=\"https://matrix.to/#/@alice:example.org\">@alice:example.org</a><br />Message replied to.</blockquote></mx-reply>Reply message."_s;
    const QString testOutputString = u"Reply message."_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
    QCOMPARE(testTextHandler.handleRecievePlainText(), testOutputString);
}

void TextHandlerTest::receiveRichInPlainOut_data()
{
    QTest::addColumn<QString>("testInputString");
    QTest::addColumn<QString>("testOutputString");

    QTest::newRow("ampersand") << u"a &amp; b"_s << u"a & b"_s;
    QTest::newRow("quote") << u"&quot;a and b&quot;"_s << u"\"a and b\""_s;
    QTest::newRow("new line") << u"new<br>line"_s << u"new\nline"_s;
    QTest::newRow("unescape") << u"can&#x27;t"_s << u"can't"_s;
}

void TextHandlerTest::receiveRichInPlainOut()
{
    QFETCH(QString, testInputString);
    QFETCH(QString, testOutputString);

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecievePlainText(Qt::RichText), testOutputString);
}

void TextHandlerTest::receivePlainTextIn()
{
    const QString testInputString = u"<plain text in tag bracket>\nTest link https://kde.org."_s;
    const QString testOutputStringRich = u"&lt;plain text in tag bracket&gt;<br>Test link <a href=\"https://kde.org\">https://kde.org</a>."_s;
    QString testOutputStringPlain = u"<plain text in tag bracket>\nTest link https://kde.org."_s;

    // Make sure quotes are maintained in a plain string.
    const QString testInputString2 = u"last line is \"Time to switch to a new topic.\""_s;
    const QString testOutputString2 = u"last line is \"Time to switch to a new topic.\""_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::PlainText), testOutputStringRich);
    QCOMPARE(testTextHandler.handleRecievePlainText(), testOutputStringPlain);

    testTextHandler.setData(testInputString2);
    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::PlainText), testOutputString2);
    QCOMPARE(testTextHandler.handleRecievePlainText(), testOutputString2);
}

void TextHandlerTest::receiveStripNewlines()
{
    const QString testInputStringPlain = u"Test\nmany\nnew\nlines."_s;
    const QString testInputStringRich = u"Test<br>many<br />new<br>lines."_s;
    const QString testOutputString = u"Test many new lines."_s;

    const QString testInputStringPlain2 = u"* List\n* Items"_s;
    const QString testOutputString2 = u"List Items"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputStringPlain);

    QCOMPARE(testTextHandler.handleRecievePlainText(Qt::PlainText, true), testOutputString);
    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::PlainText, nullptr, nullptr, true), testOutputString);

    testTextHandler.setData(testInputStringRich);
    QCOMPARE(testTextHandler.handleRecievePlainText(Qt::RichText, true), testOutputString);
    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText, nullptr, nullptr, true), testOutputString);

    testTextHandler.setData(testInputStringPlain2);
    QCOMPARE(testTextHandler.handleRecievePlainText(Qt::RichText, true), testOutputString2);
}

/**
 * For a plain text output of a received string all html is stripped except for
 * code which is unescaped if it's html.
 */
void TextHandlerTest::receivePlainStripHtml()
{
    const QString testInputString = u"<p>Test</p> <pre><code>Some code <strong>with tags</strong></code></pre>"_s;
    const QString testOutputString = u"Test Some code <strong>with tags</strong>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecievePlainText(Qt::RichText), testOutputString);
}

void TextHandlerTest::receivePlainStripMarkup()
{
    const QString testInputString = u"**bold** `<p>inline code</p>` *italic*"_s;
    const QString testOutputString = u"bold <p>inline code</p> italic"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecievePlainText(), testOutputString);
}

void TextHandlerTest::receiveRichUserPill()
{
    const QString testInputString = u"<p><a href=\"https://matrix.to/#/@alice:example.org\">@alice:example.org</a></p>"_s;
    const QString testOutputString = u"<b><a href=\"https://matrix.to/#/@alice:example.org\">@alice:example.org</a></b>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::receiveRichStrikethrough()
{
    const QString testInputString = u"<p><del>Test</del></p>"_s;
    const QString testOutputString = u"<s>Test</s>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::receiveRichtextIn()
{
    const QString testInputString = u"<p>Test</p> <pre><code>Some code <strong>with tags</strong></code></pre>"_s;
    const QString testOutputString = u"<p>Test</p> <pre><code>Some code &lt;strong&gt;with tags&lt;/strong&gt;</code></pre>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::receiveRichMxcUrl()
{
    const QString testInputString =
        u"<img src=\"mxc://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e\" alt=\"image\"><img src=\"mxc://kde.org/34c3464b3a1bd7f55af2d559e07d2c773c430e73\" alt=\"image\">"_s;
    const QString testOutputString =
        u"<img src=\"mxc://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e?user_id=@bob:kde.org&room_id=%23myroom:kde.org&event_id=$143273582443PhrSn:example.org\" alt=\"image\"><img src=\"mxc://kde.org/34c3464b3a1bd7f55af2d559e07d2c773c430e73?user_id=@bob:kde.org&room_id=%23myroom:kde.org&event_id=$143273582443PhrSn:example.org\" alt=\"image\">"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText, room, room->messageEvents().at(0).get()), testOutputString);
}

void TextHandlerTest::receiveRichPlainUrl_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");

    // This is an actual link that caused trouble which is why it's so long. Keeping
    // so we can confirm consistent behaviour for complex urls.
    QTest::addRow("link 1")
        << u"https://matrix.to/#/!RvzunyTWZGfNxJVQqv:matrix.org/$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im <a href=\"https://matrix.to/#/!RvzunyTWZGfNxJVQqv:matrix.org/$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im\">Link already rich</a>"_s
        << u"<a href=\"https://matrix.to/#/!RvzunyTWZGfNxJVQqv:matrix.org/$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im\">https://matrix.to/#/!RvzunyTWZGfNxJVQqv:matrix.org/$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im</a> <a href=\"https://matrix.to/#/!RvzunyTWZGfNxJVQqv:matrix.org/$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im\">Link already rich</a>"_s;

    // Another real case. The linkification wasn't handling it when a single link
    // contains what looks like and email. It was broken into 3 but needs to
    // be just single link.
    QTest::addRow("link 2")
        << u"https://lore.kernel.org/lkml/CAHk-=wio46vC4t6xXD-sFqjoPwFm_u515jm3suzmkGxQTeA1_A@mail.gmail.com/"_s
        << u"<a href=\"https://lore.kernel.org/lkml/CAHk-=wio46vC4t6xXD-sFqjoPwFm_u515jm3suzmkGxQTeA1_A@mail.gmail.com/\">https://lore.kernel.org/lkml/CAHk-=wio46vC4t6xXD-sFqjoPwFm_u515jm3suzmkGxQTeA1_A@mail.gmail.com/</a>"_s;

    QTest::addRow("email") << uR"(email@example.com <a href="mailto:email@example.com">Link already rich</a>)"_s
                           << uR"(<a href="mailto:email@example.com">email@example.com</a> <a href="mailto:email@example.com">Link already rich</a>)"_s;
    QTest::addRow("mxid")
        << u"@user:kde.org <a href=\"https://matrix.to/#/@user:kde.org\">Link already rich</a>"_s
        << u"<b><a href=\"https://matrix.to/#/@user:kde.org\">@user:kde.org</a></b> <b><a href=\"https://matrix.to/#/@user:kde.org\">Link already rich</a></b>"_s;
    QTest::addRow("mxid with prefix") << u"a @user:kde.org b"_s << u"a <b><a href=\"https://matrix.to/#/@user:kde.org\">@user:kde.org</a></b> b"_s;
}

/**
 * For when your rich input string has a plain text url left in.
 *
 * This test is to show that a url that is already rich will be left alone but a
 * plain one will be linkified.
 */
void TextHandlerTest::receiveRichPlainUrl()
{
    QFETCH(QString, input);
    QFETCH(QString, output);

    TextHandler testTextHandler;
    testTextHandler.setData(input);

    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText), output);
}

void TextHandlerTest::receiveRichEdited_data()
{
    QTest::addColumn<QString>("testInputString");
    QTest::addColumn<QString>("testOutputString");

    auto theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));

    QTest::newRow("basic") << u"Edited"_s << u"Edited <span style=\"color:%1\">(edited)</span>"_s.arg(theme ? theme->disabledTextColor().name() : u"#000000"_s);
    QTest::newRow("multiple paragraphs") << u"<p>Edited</p>\n<p>Edited</p>"_s
                                         << u"<p>Edited</p>\n<p>Edited <span style=\"color:%1\">(edited)</span></p>"_s.arg(
                                                theme ? theme->disabledTextColor().name() : u"#000000"_s);
}

void TextHandlerTest::receiveRichEdited()
{
    QFETCH(QString, testInputString);
    QFETCH(QString, testOutputString);

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    const auto event = eventCast<const Quotient::RoomMessageEvent>(room->messageEvents().at(2).get());
    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText, room, event, false, event->isReplaced()), testOutputString);
}

void TextHandlerTest::receiveLineSeparator()
{
    auto text = u"foo\u2028bar"_s;
    TextHandler textHandler;
    textHandler.setData(text);
    QCOMPARE(textHandler.handleRecievePlainText(Qt::PlainText, true), u"foo bar"_s);
}

void TextHandlerTest::receiveRichCodeUrl()
{
    auto input = u"<code>https://kde.org</code>"_s;
    TextHandler testTextHandler;
    testTextHandler.setData(input);
    QCOMPARE(testTextHandler.handleRecieveRichText(), input);
}

void TextHandlerTest::receiveRichColor()
{
    const QString testInputString =
        u"<span data-mx-color=\"#ff00be\">¯</span><span data-mx-color=\"#ff3b1d\">\\</span><span data-mx-color=\"#ffa600\">_</span><span data-mx-color=\"#64d200\">(</span><span data-mx-color=\"#00e261\">ツ</span><span data-mx-color=\"#00e7ff\">)</span><span data-mx-color=\"#00e1ff\">_</span><span data-mx-color=\"#00bdff\">/</span><span data-mx-color=\"#ff60ff\">¯</span>"_s;
    const QString testOutputString =
        u"<span style=\"color: #ff00be;\">¯</span><span style=\"color: #ff3b1d;\">\\</span><span style=\"color: #ffa600;\">_</span><span style=\"color: #64d200;\">(</span><span style=\"color: #00e261;\">ツ</span><span style=\"color: #00e7ff;\">)</span><span style=\"color: #00e1ff;\">_</span><span style=\"color: #00bdff;\">/</span><span style=\"color: #ff60ff;\">¯</span>"_s;

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::componentOutput_data()
{
    QTest::addColumn<QString>("testInputString");
    QTest::addColumn<QList<MessageComponent>>("testOutputComponents");

    QTest::newRow("multiple paragraphs") << u"<p>Text</p>\n<p>Text</p>"_s
                                         << QList<MessageComponent>{MessageComponent{MessageComponentType::Text, u"Text"_s, {}},
                                                                    MessageComponent{MessageComponentType::Text, u"Text"_s, {}}};
    QTest::newRow("code") << u"<p>Text</p>\n<pre><code class=\"language-html\">Some code\n</code></pre>"_s
                          << QList<MessageComponent>{MessageComponent{MessageComponentType::Text, u"Text"_s, {}},
                                                     MessageComponent{MessageComponentType::Code, u"Some code"_s, QVariantMap{{u"class"_s, u"html"_s}}}};
    QTest::newRow("quote") << u"<p>Text</p>\n<blockquote>\n<p>blockquote</p>\n</blockquote>"_s
                           << QList<MessageComponent>{MessageComponent{MessageComponentType::Text, u"Text"_s, {}},
                                                      MessageComponent{MessageComponentType::Quote, u"“blockquote”"_s, {}}};
    QTest::newRow("multiple paragraph quote") << u"<blockquote>\n<p>blockquote</p>\n<p>next paragraph</p>\n</blockquote>"_s
                                              << QList<MessageComponent>{
                                                     MessageComponent{MessageComponentType::Quote, u"<p>“blockquote</p>\n<p>next paragraph”</p>"_s, {}}};
    QTest::newRow("no tag first paragraph") << u"Text\n<p>Text</p>"_s
                                            << QList<MessageComponent>{MessageComponent{MessageComponentType::Text, u"Text"_s, {}},
                                                                       MessageComponent{MessageComponentType::Text, u"Text"_s, {}}};
    QTest::newRow("no tag last paragraph") << u"<p>Text</p>\nText"_s
                                           << QList<MessageComponent>{MessageComponent{MessageComponentType::Text, u"Text"_s, {}},
                                                                      MessageComponent{MessageComponentType::Text, u"Text"_s, {}}};
    QTest::newRow("inline code") << u"<p><code>https://kde.org</code></p>\n<p>Text</p>"_s
                                 << QList<MessageComponent>{MessageComponent{MessageComponentType::Text, u"<code>https://kde.org</code>"_s, {}},
                                                            MessageComponent{MessageComponentType::Text, u"Text"_s, {}}};
    QTest::newRow("inline code single block") << u"<code>https://kde.org</code>"_s
                                              << QList<MessageComponent>{MessageComponent{MessageComponentType::Text, u"<code>https://kde.org</code>"_s, {}}};
    QTest::newRow("long start tag")
        << u"Ah, you mean something like<br/><pre data-md=\"```\"><code class=\"language-qml\"># main.qml\nimport CustomQml\n...\nControls.TextField { id: someField }\nCustomQml {\n    someTextProperty: someField.text\n}\n</code></pre>Sure you can, it's still local to the same file where you defined the id"_s
        << QList<MessageComponent>{
               MessageComponent{MessageComponentType::Text, u"Ah, you mean something like<br/>"_s, {}},
               MessageComponent{
                   MessageComponentType::Code,
                   u"# main.qml\nimport CustomQml\n...\nControls.TextField { id: someField }\nCustomQml {\n    someTextProperty: someField.text\n}"_s,
                   QVariantMap{{u"class"_s, u"qml"_s}}},
               MessageComponent{MessageComponentType::Text, u"Sure you can, it's still local to the same file where you defined the id"_s, {}}};
}

void TextHandlerTest::componentOutput()
{
    QFETCH(QString, testInputString);
    QFETCH(QList<MessageComponent>, testOutputComponents);

    TextHandler testTextHandler;
    QCOMPARE(testTextHandler.textComponents(testInputString), testOutputComponents);
}

QTEST_MAIN(TextHandlerTest)
#include "texthandlertest.moc"
