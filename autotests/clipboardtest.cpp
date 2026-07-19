// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include <QClipboard>
#include <QMimeData>

#include "clipboard.h"

using namespace Qt::StringLiterals;

class ClipboardTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void plainText();
    void richText();
    void markdownText();
};

void ClipboardTest::plainText()
{
    auto qClipboard = QGuiApplication::clipboard();
    auto clipboard = Clipboard();
    qClipboard->setText(u"plain text"_s);
    QCOMPARE(clipboard.plainText(), u"plain text"_s);

    auto richMime = new QMimeData();
    richMime->setHtml(u"<b>rich text</b>"_s);
    qClipboard->setMimeData(richMime);
    QCOMPARE(clipboard.plainText(), u"rich text"_s);
}

void ClipboardTest::richText()
{
    auto qClipboard = QGuiApplication::clipboard();
    auto clipboard = Clipboard();
    qClipboard->setText(u"plain text"_s);
    QCOMPARE(clipboard.richText(Clipboard::PlainToRich), u"plain text"_s);
    QCOMPARE(clipboard.richText(Clipboard::Raw), u""_s);

    qClipboard->setText(u"plain text\nnew line"_s);
    QCOMPARE(clipboard.richText(Clipboard::PlainToRich), u"plain text<br>new line"_s);

    auto richMime = new QMimeData();
    richMime->setHtml(u"<b>rich text</b>"_s);
    qClipboard->setMimeData(richMime);
    QCOMPARE(clipboard.richText(Clipboard::PlainToRich), u"<b>rich text</b>"_s);
}

void ClipboardTest::markdownText()
{
    auto qClipboard = QGuiApplication::clipboard();
    auto clipboard = Clipboard();
    qClipboard->setText(u"**plain text**"_s);
    QCOMPARE(clipboard.richText(Clipboard::PlainToRich), u"**plain text**"_s);
    QCOMPARE(clipboard.richText(Clipboard::PlainToRich), u"**plain text**"_s);
    QCOMPARE(clipboard.richText(Clipboard::ConvertMarkdown), u"<strong>plain text</strong>"_s);
}

QTEST_MAIN(ClipboardTest)
#include "clipboardtest.moc"
