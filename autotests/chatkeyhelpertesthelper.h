// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QQuickTextDocument>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <qquickitem.h>

#include "chatkeyhelper.h"
#include "chattextitemhelper.h"

class ChatKeyHelperTestHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(ChatTextItemHelper *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    Q_PROPERTY(ChatKeyHelper *keyHelper READ keyHelper CONSTANT)

public:
    explicit ChatKeyHelperTestHelper(QObject *parent = nullptr)
        : QObject(parent)
        , m_keyHelper(new ChatKeyHelper(this))
    {
    }

    ChatTextItemHelper *textItem() const
    {
        return m_textItem;
    }
    void setTextItem(ChatTextItemHelper *textItem)
    {
        if (textItem == m_textItem) {
            return;
        }
        m_textItem = textItem;
        m_keyHelper->setTextItem(textItem);
        Q_EMIT textItemChanged();
    }

    ChatKeyHelper *keyHelper() const
    {
        return m_keyHelper;
    }

Q_SIGNALS:
    void textItemChanged();

private:
    QPointer<ChatTextItemHelper> m_textItem;
    QPointer<ChatKeyHelper> m_keyHelper;
};
