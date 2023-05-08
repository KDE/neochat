// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "texthandler.h"

#include <qnamespace.h>
#include <quotient_common.h>
#include <syncdata.h>

using namespace Quotient;

class TestRoom : public NeoChatRoom
{
public:
    using NeoChatRoom::NeoChatRoom;

    void update(SyncRoomData &&data, bool fromCache = false)
    {
        Room::updateData(std::move(data), fromCache);
    }
};

class TextHandlerTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestRoom *room = nullptr;

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
    void receiveRichPlainUrl();
    void receiveRichEmote();
    void receiveRichEdited_data();
    void receiveRichEdited();
    void receiveLineSeparator();

    void linkPreviewsMatch_data();
    void linkPreviewsMatch();
    void linkPreviewsReject_data();
    void linkPreviewsReject();
};

#ifdef QUOTIENT_07
void TextHandlerTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
    room = new TestRoom(connection, QStringLiteral("#myroom:kde.org"), JoinState::Join);

    const auto json = QJsonDocument::fromJson(R"EVENT({
  "account_data": {
    "events": [
      {
        "content": {
          "tags": {
            "u.work": {
              "order": 0.9
            }
          }
        },
        "type": "m.tag"
      },
      {
        "content": {
          "custom_config_key": "custom_config_value"
        },
        "type": "org.example.custom.room.config"
      }
    ]
  },
  "ephemeral": {
    "events": [
      {
        "content": {
          "user_ids": [
            "@alice:matrix.org",
            "@bob:example.com"
          ]
        },
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "type": "m.typing"
      }
    ]
  },
  "state": {
    "events": [
      {
        "content": {
          "avatar_url": "mxc://example.org/SEsfnsuifSDFSSEF",
          "displayname": "Alice Margatroid",
          "membership": "join",
          "reason": "Looking for support"
        },
        "event_id": "$143273582443PhrSn:example.org",
        "origin_server_ts": 1432735824653,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "state_key": "@alice:example.org",
        "type": "m.room.member",
        "unsigned": {
          "age": 1234
        }
      }
    ]
  },
  "summary": {
    "m.heroes": [
      "@alice:example.com",
      "@bob:example.com"
    ],
    "m.invited_member_count": 0,
    "m.joined_member_count": 2
  },
  "timeline": {
    "events": [
      {
        "content": {
          "body": "This is an **example** text message",
          "format": "org.matrix.custom.html",
          "formatted_body": "<b>This is an example text message</b>",
          "msgtype": "m.text"
        },
        "event_id": "$143273582443PhrSn:example.org",
        "origin_server_ts": 1432735824654,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "type": "m.room.message",
        "unsigned": {
          "age": 1232
        }
      },
      {
        "content": {
          "body": "/me This is an emote.",
          "format": "org.matrix.custom.html",
          "formatted_body": "This is an emote.",
          "msgtype": "m.emote"
        },
        "event_id": "$153273582443PhrSn:example.org",
        "origin_server_ts": 1532735824654,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "type": "m.room.message",
        "unsigned": {
          "age": 1231
        }
      },
      {
        "content": {
            "body": "tested",
            "msgtype": "m.text"
        },
        "event_id": "$zrCiBxBnqqTn0Z5FY78qSZAszno_w8nJJXzfBULG-3E",
        "origin_server_ts": 1680948575928,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "type": "m.room.message",
        "unsigned": {
            "age": 1747776,
            "m.relations": {
                "m.replace": {
                    "event_id": "$UX0PlpyI7vYO32iHMuuYEP7ECMh4sX3XLGiB2SwM4mQ",
                    "origin_server_ts": 1680948580992,
                    "sender": "@example:example.org"
                }
            }
        }
      }
    ],
    "limited": true,
    "prev_batch": "t34-23535_0_0"
  }
})EVENT");
    SyncRoomData roomData(QStringLiteral("@bob:kde.org"), JoinState::Join, json.object());
    room->update(std::move(roomData));
}
#endif

void TextHandlerTest::allowedAttributes()
{
    const QString testInputString1 = QStringLiteral("<p><span data-mx-spoiler><font color=#FFFFFF>Test</font><span></p>");
    const QString testOutputString1 = QStringLiteral("<p><span data-mx-spoiler><font color=#FFFFFF>Test</font><span></p>");
    // Handle urls where the href has either single (') or double (") quotes.
    const QString testInputString2 = QStringLiteral("<p><a href=\"https://kde.org\">link</a><a href='https://kde.org'>link</a></p>");
    const QString testOutputString2 = QStringLiteral("<p><a href=\"https://kde.org\">link</a><a href='https://kde.org'>link</a></p>");

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
    const QString testInputString = QStringLiteral("<p>Allowed</p> <span>Allowed</span> <body>Disallowed</body>");
    const QString testOutputString = QStringLiteral("<p>Allowed</p> <span>Allowed</span> Disallowed");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::stripDisallowedAttributes()
{
    const QString testInputString = QStringLiteral("<p style=\"font-size:50px;\" color=#FFFFFF>Test</p>");
    const QString testOutputString = QStringLiteral("<p>Test</p>");

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
    const QString testInputString = QStringLiteral("<pre><code></code></pre>");
    const QString testOutputString = QStringLiteral("<pre><code></code></pre>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::sendSimpleStringCase()
{
    const QString testInputString = QStringLiteral("This data should just be put in a paragraph.");
    const QString testOutputString = QStringLiteral("<p>This data should just be put in a paragraph.</p>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendSingleParaMarkup()
{
    const QString testInputString = QStringLiteral(
        "Text para with **bold**, *italic*, [link](https://kde.org), ![image](mxc://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e), `inline code`.");
    const QString testOutputString = QStringLiteral(
        "<p>Text para with <strong>bold</strong>, <em>italic</em>, <a href=\"https://kde.org\">link</a>, <img "
        "src=\"mxc://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e\" alt=\"image\">, <code>inline code</code>.</p>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendMultipleSectionMarkup()
{
    const QString testInputString =
        QStringLiteral("Text para\n> blockquote\n* List 1\n* List 2\n1. one\n2. two\n# Heading 1\n## Heading 2\nhorizontal rule\n\n---\n```\ncodeblock\n```");
    const QString testOutputString = QStringLiteral(
        "<p>Text para</p>\n<blockquote>\n<p>blockquote</p>\n</blockquote>\n<ul>\n<li>List 1</li>\n<li>List "
        "2</li>\n</ul>\n<ol>\n<li>one</li>\n<li>two</li>\n</ol>\n<h1>Heading 1</h1>\n<h2>Heading 2</h2>\n<p>horizontal "
        "rule</p>\n<hr>\n<pre><code>codeblock\n</code></pre>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendBadLinks()
{
    const QString testInputString = QStringLiteral("[link](kde.org), ![image](https://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e)");
    const QString testOutputString = QStringLiteral("<p><a>link</a>, <img alt=\"image\"></p>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

/**
 * All text between code tags is treated as plain so it should get escaped.
 */
void TextHandlerTest::sendEscapeCode()
{
    const QString testInputString = QStringLiteral("```\n<p>Test <span style=\"font-size:50px;\">some</span> code</p>\n```");
    const QString testOutputString =
        QStringLiteral("<pre><code>&lt;p&gt;Test &lt;span style=&quot;font-size:50px;&quot;&gt;some&lt;/span&gt; code&lt;/p&gt;\n</code></pre>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::sendCodeClass()
{
    const QString testInputString = QStringLiteral("```html\nsome code\n```\n<pre><code class=\"code-underline\">some more code</code></pre>");
    const QString testOutputString = QStringLiteral("<pre><code class=\"language-html\">some code\n</code></pre>\n<pre><code>some more code</code></pre>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleSendText(), testOutputString);
}

void TextHandlerTest::receiveStripReply()
{
    const QString testInputString = QStringLiteral(
        "<mx-reply><blockquote><a href=\"https://matrix.to/#/!somewhere:example.org/$event:example.org\">In reply to</a><a "
        "href=\"https://matrix.to/#/@alice:example.org\">@alice:example.org</a><br />Message replied to.</blockquote></mx-reply>Reply message.");
    const QString testOutputString = QStringLiteral("Reply message.");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
    QCOMPARE(testTextHandler.handleRecievePlainText(), testOutputString);
}

void TextHandlerTest::receiveRichInPlainOut_data()
{
    QTest::addColumn<QString>("testInputString");
    QTest::addColumn<QString>("testOutputString");

    QTest::newRow("ampersand") << QStringLiteral("a &amp; b") << QStringLiteral("a & b");
    QTest::newRow("quote") << QStringLiteral("&quot;a and b&quot;") << QStringLiteral("\"a and b\"");
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
    const QString testInputString = QStringLiteral("<plain text in tag bracket>\nTest link https://kde.org.");
    const QString testOutputStringRich = QStringLiteral("&lt;plain text in tag bracket&gt;<br>Test link <a href=\"https://kde.org\">https://kde.org</a>.");
    QString testOutputStringPlain = QStringLiteral("<plain text in tag bracket>\nTest link https://kde.org.");

    // Make sure quotes are maintained in a plain string.
    const QString testInputString2 = QStringLiteral("last line is \"Time to switch to a new topic.\"");
    const QString testOutputString2 = QStringLiteral("last line is \"Time to switch to a new topic.\"");

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
    const QString testInputStringPlain = QStringLiteral("Test\nmany\nnew\nlines.");
    const QString testInputStringRich = QStringLiteral("Test<br>many<br />new<br>lines.");
    const QString testOutputString = QStringLiteral("Test many new lines.");

    const QString testInputStringPlain2 = QStringLiteral("* List\n* Items");
    const QString testOutputString2 = QStringLiteral("List Items");

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
    const QString testInputString = QStringLiteral("<p>Test</p> <pre><code>Some code <strong>with tags</strong></code></pre>");
    const QString testOutputString = QStringLiteral("Test Some code <strong>with tags</strong>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecievePlainText(Qt::RichText), testOutputString);
}

void TextHandlerTest::receivePlainStripMarkup()
{
    const QString testInputString = QStringLiteral("**bold** `<p>inline code</p>` *italic*");
    const QString testOutputString = QStringLiteral("bold <p>inline code</p> italic");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecievePlainText(), testOutputString);
}

void TextHandlerTest::receiveRichUserPill()
{
    const QString testInputString = QStringLiteral("<p><a href=\"https://matrix.to/#/@alice:example.org\">@alice:example.org</a></p>");
    const QString testOutputString = QStringLiteral("<p><b><a href=\"https://matrix.to/#/@alice:example.org\">@alice:example.org</a></b></p>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::receiveRichStrikethrough()
{
    const QString testInputString = QStringLiteral("<p><del>Test</del></p>");
    const QString testOutputString = QStringLiteral("<p><s>Test</s></p>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

void TextHandlerTest::receiveRichtextIn()
{
    const QString testInputString = QStringLiteral("<p>Test</p> <pre><code>Some code <strong>with tags</strong></code></pre>");
    const QString testOutputString = QStringLiteral("<p>Test</p> <pre><code>Some code &lt;strong&gt;with tags&lt;/strong&gt;</code></pre>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(), testOutputString);
}

#ifdef QUOTIENT_07
void TextHandlerTest::receiveRichMxcUrl()
{
    const QString testInputString = QStringLiteral(
        "<img src=\"mxc://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e\" alt=\"image\"><img src=\"mxc://kde.org/34c3464b3a1bd7f55af2d559e07d2c773c430e73\" "
        "alt=\"image\">");
    const QString testOutputString = QStringLiteral(
        "<img "
        "src=\"mxc://kde.org/aebd3ffd40503e1ef0525bf8f0d60282fec6183e?user_id=@bob:kde.org&room_id=%23myroom:kde.org&event_id=$143273582443PhrSn:example.org\" "
        "alt=\"image\"><img "
        "src=\"mxc://kde.org/34c3464b3a1bd7f55af2d559e07d2c773c430e73?user_id=@bob:kde.org&room_id=%23myroom:kde.org&event_id=$143273582443PhrSn:example.org\" "
        "alt=\"image\">");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText, room, room->messageEvents().at(0).get()), testOutputString);
}
#endif

/**
 * For when your rich input string has a plain text url left in.
 *
 * This test is to show that a url that is already rich will be left alone but a
 * plain one will be linkified.
 */
void TextHandlerTest::receiveRichPlainUrl()
{
    // This is an actual link that caused trouble which is why it's so long. Keeping
    // so we can confirm consistent behaviour for complex urls.
    const QString testInputStringLink1 = QStringLiteral(
        "https://matrix.to/#/!RvzunyTWZGfNxJVQqv:matrix.org/$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im "
        "<a "
        "href=\"https://matrix.to/#/!RvzunyTWZGfNxJVQqv:matrix.org/"
        "$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im\">Link already rich</a>");
    const QString testOutputStringLink1 = QStringLiteral(
        "<a "
        "href=\"https://matrix.to/#/!RvzunyTWZGfNxJVQqv:matrix.org/"
        "$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im\">https://matrix.to/#/"
        "!RvzunyTWZGfNxJVQqv:matrix.org/$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im</a> <a "
        "href=\"https://matrix.to/#/!RvzunyTWZGfNxJVQqv:matrix.org/"
        "$-9TJVTh5PvW6MvIhFDwteiyLBVGriinueO5eeIazQS8?via=libera.chat&amp;via=matrix.org&amp;via=fedora.im\">Link already rich</a>");

    // Another real case. The linkification wasn't handling it when a single link
    // contains what looks like and email. It was been broken into 3 but needs to
    // be just single link.
    const QString testInputStringLink2 = QStringLiteral("https://lore.kernel.org/lkml/CAHk-=wio46vC4t6xXD-sFqjoPwFm_u515jm3suzmkGxQTeA1_A@mail.gmail.com/");
    const QString testOutputStringLink2 = QStringLiteral(
        "<a "
        "href=\"https://lore.kernel.org/lkml/CAHk-=wio46vC4t6xXD-sFqjoPwFm_u515jm3suzmkGxQTeA1_A@mail.gmail.com/\">https://lore.kernel.org/lkml/"
        "CAHk-=wio46vC4t6xXD-sFqjoPwFm_u515jm3suzmkGxQTeA1_A@mail.gmail.com/</a>");

    QString testInputStringEmail = QStringLiteral(R"(email@example.com <a href="mailto:email@example.com">Link already rich</a>)");
    QString testOutputStringEmail =
        QStringLiteral(R"(<a href="mailto:email@example.com">email@example.com</a> <a href="mailto:email@example.com">Link already rich</a>)");

    QString testInputStringMxId = QStringLiteral("@user:kde.org <a href=\"https://matrix.to/#/@user:kde.org\">Link already rich</a>");
    QString testOutputStringMxId = QStringLiteral(
        "<b><a href=\"https://matrix.to/#/@user:kde.org\">@user:kde.org</a></b> <b><a href=\"https://matrix.to/#/@user:kde.org\">Link already rich</a></b>");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputStringLink1);

    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText), testOutputStringLink1);

    testTextHandler.setData(testInputStringLink2);
    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText), testOutputStringLink2);

    testTextHandler.setData(testInputStringEmail);
    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText), testOutputStringEmail);

    testTextHandler.setData(testInputStringMxId);
    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText), testOutputStringMxId);
}

// Test that user pill is add to an emote message.
// N.B. The second message in the test timeline is marked as an emote.
void TextHandlerTest::receiveRichEmote()
{
    auto event = room->messageEvents().at(1).get();
    auto author = static_cast<NeoChatUser *>(room->user(event->senderId()));
    const QString testInputString = QStringLiteral("This is an emote.");
    const QString testOutputString = QStringLiteral("* <a href=\"https://matrix.to/#/@example:example.org\" style=\"color:") + author->color().name()
        + QStringLiteral("\">@example:example.org</a> This is an emote.");

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText, room, event), testOutputString);
}

void TextHandlerTest::receiveRichEdited_data()
{
    QTest::addColumn<QString>("testInputString");
    QTest::addColumn<QString>("testOutputString");

    QTest::newRow("basic") << QStringLiteral("Edited") << QStringLiteral("Edited <span style=\"color:#000000\">(edited)</span>");
    QTest::newRow("multiple paragraphs") << QStringLiteral("<p>Edited</p>\n<p>Edited</p>")
                                         << QStringLiteral("<p>Edited</p>\n<p>Edited <span style=\"color:#000000\">(edited)</span></p>");
    QTest::newRow("blockquote") << QStringLiteral("<blockquote>Edited</blockquote>")
                                << QStringLiteral("<blockquote>Edited</blockquote><p> <span style=\"color:#000000\">(edited)</span></p>");
}

void TextHandlerTest::receiveRichEdited()
{
    QFETCH(QString, testInputString);
    QFETCH(QString, testOutputString);

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.handleRecieveRichText(Qt::RichText, room, room->messageEvents().at(2).get()), testOutputString);
}

void TextHandlerTest::receiveLineSeparator()
{
    auto text = QStringLiteral("foo\u2028bar");
    TextHandler textHandler;
    textHandler.setData(text);
    QCOMPARE(textHandler.handleRecievePlainText(Qt::PlainText, true), QStringLiteral("foo bar"));
}

void TextHandlerTest::linkPreviewsMatch_data()
{
    QTest::addColumn<QString>("testInputString");
    QTest::addColumn<QList<QUrl>>("testOutputLinks");

    QTest::newRow("plainHttps") << QStringLiteral("https://kde.org") << QList<QUrl>({QUrl("https://kde.org")});
    QTest::newRow("richHttps") << QStringLiteral("<a href=\"https://kde.org\">Rich Link</a>") << QList<QUrl>({QUrl("https://kde.org")});
    QTest::newRow("plainWww") << QStringLiteral("www.example.org") << QList<QUrl>({QUrl("www.example.org")});
    QTest::newRow("multipleHttps") << QStringLiteral("https://kde.org www.example.org")
                                   << QList<QUrl>({
                                          QUrl("https://kde.org"),
                                          QUrl("www.example.org"),
                                      });
}

void TextHandlerTest::linkPreviewsMatch()
{
    QFETCH(QString, testInputString);
    QFETCH(QList<QUrl>, testOutputLinks);

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.getLinkPreviews(), testOutputLinks);
}

void TextHandlerTest::linkPreviewsReject_data()
{
    QTest::addColumn<QString>("testInputString");
    QTest::addColumn<QList<QUrl>>("testOutputLinks");

    QTest::newRow("mxc") << QStringLiteral("mxc://example.org/SEsfnsuifSDFSSEF") << QList<QUrl>();
    QTest::newRow("matrixTo") << QStringLiteral("https://matrix.to/#/@alice:example.org") << QList<QUrl>();
    QTest::newRow("noSpace") << QStringLiteral("testhttps://kde.org") << QList<QUrl>();
}

void TextHandlerTest::linkPreviewsReject()
{
    QFETCH(QString, testInputString);
    QFETCH(QList<QUrl>, testOutputLinks);

    TextHandler testTextHandler;
    testTextHandler.setData(testInputString);

    QCOMPARE(testTextHandler.getLinkPreviews(), testOutputLinks);
}

QTEST_GUILESS_MAIN(TextHandlerTest)
#include "texthandlertest.moc"
