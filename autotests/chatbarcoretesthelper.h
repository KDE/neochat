// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QTextCursor>

#include <Quotient/connection.h>

#include "testutils.h"

class ChatBarCoreTestHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isWindows READ isWindows CONSTANT)

    /**
     * @brief The QML text Item the ChatMerkdownHelper is handling.
     */
    Q_PROPERTY(NeoChatRoom *room READ room CONSTANT)

public:
    explicit ChatBarCoreTestHelper(QObject *parent = nullptr)
        : QObject(parent)
        , m_connection(Quotient::Connection::makeMockConnection(u"@bob:kde.org"_s))
        , m_room(new TestUtils::TestRoom(m_connection, u"#myroom:kde.org"_s, u"test-eventhandler-sync.json"_s))
    {
        if (m_connection) {
            m_connection->setParent(this);
        }
    }

    bool isWindows() const
    {
#ifdef Q_OS_WINDOWS
        return true;
#else
        return false;
#endif
    }

    NeoChatRoom *room() const
    {
        return m_room;
    }

    Q_INVOKABLE QString cacheString(Blocks::Cache *cache)
    {
        if (!cache) {
            return {};
        }
        return cache->toString();
    }

    Q_INVOKABLE QString cachePlainString(Blocks::Cache *cache)
    {
        if (!cache) {
            return {};
        }
        return cache->toPlainString();
    }

private:
    Quotient::Connection *m_connection;
    QPointer<TestUtils::TestRoom> m_room;
};
