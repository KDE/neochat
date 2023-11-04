// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "chatdocumenthandler.h"
#include "neochatconfig.h"

class ChatDocumentHandlerTest : public QObject
{
    Q_OBJECT

private:
    ChatDocumentHandler emptyHandler;

private Q_SLOTS:
    void initTestCase();

    void nullComplete();
};

void ChatDocumentHandlerTest::initTestCase()
{
    // HACK: this is to stop KStatusNotifierItem SEGFAULTING on cleanup.
    NeoChatConfig::self()->setSystemTray(false);
}

void ChatDocumentHandlerTest::nullComplete()
{
    QTest::ignoreMessage(QtWarningMsg, "complete called with m_document set to nullptr.");
    emptyHandler.complete(0);
}

QTEST_MAIN(ChatDocumentHandlerTest)
#include "chatdocumenthandlertest.moc"
