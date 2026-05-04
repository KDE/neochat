// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QQuickTextDocument>
#include <QTextCursor>
#include <QTextDocumentFragment>

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
        return m_keyHelper->textItem();
    }

    void setTextItem(ChatTextItemHelper *textItem)
    {
        if (textItem == m_keyHelper->textItem()) {
            return;
        }
        m_keyHelper->setTextItem(textItem);
        Q_EMIT textItemChanged();
    }

    ChatKeyHelper *keyHelper() const
    {
        return m_keyHelper;
    }

    Q_INVOKABLE void insertLink(const QString &linkUrl, const QString &linkText)
    {
        if (!m_keyHelper || !m_keyHelper->textItem()) {
            return;
        }
        auto cursor = m_keyHelper->textItem()->textCursor();
        if (cursor.isNull()) {
            return;
        }
        const auto originalFormat = cursor.charFormat();
        auto format = cursor.charFormat();
        format.setAnchor(true);
        format.setAnchorHref(linkUrl);
        cursor.insertText(linkText, format);
        cursor.insertText(u" "_s, originalFormat);
    }

Q_SIGNALS:
    void textItemChanged();

private:
    QPointer<ChatKeyHelper> m_keyHelper;
};
