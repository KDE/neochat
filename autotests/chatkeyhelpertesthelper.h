// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QClipboard>
#include <QGuiApplication>
#include <QObject>
#include <QQuickItem>
#include <QQuickTextDocument>
#include <QTextCursor>
#include <QTextDocumentFragment>

#include "block.h"
#include "chatkeyhelper.h"
#include "chattextitemhelper.h"
#include "texthandler.h"

class ChatKeyHelperTestHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isWindows READ isWindows CONSTANT)

    Q_PROPERTY(ChatTextItemHelper *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    Q_PROPERTY(ChatKeyHelper *keyHelper READ keyHelper CONSTANT)

    Q_PROPERTY(bool richTextActive READ richTextActive WRITE setRichTextActive)

public:
    explicit ChatKeyHelperTestHelper(QObject *parent = nullptr)
        : QObject(parent)
        , m_keyHelper(new ChatKeyHelper(this))
    {
    }

    bool isWindows() const
    {
#ifdef Q_OS_WINDOWS
        return true;
#else
        return false;
#endif
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

    bool richTextActive() const
    {
        return ChatKeyHelper::richTextActive;
    }

    void setRichTextActive(bool richTextActive)
    {
        ChatKeyHelper::richTextActive = richTextActive;
    }

    Q_INVOKABLE void setClipboardText(const QString &text)
    {
        QGuiApplication::clipboard()->setText(text);
    }

    Q_INVOKABLE void clearClipboard()
    {
        QGuiApplication::clipboard()->clear();
    }

    /**
     * Split the given HTML into blocks like the chat bar does for pasted text and return the plain text of each block.
     */
    Q_INVOKABLE QStringList blocksPlainText(const QString &html) const
    {
        QStringList result;
        for (const auto block : TextHandler().textComponents(html)) {
            if (const auto textBlock = dynamic_cast<Blocks::TextBlock *>(block)) {
                result += textBlock->item()->initialFragment().toPlainText();
            }
            delete block;
        }
        return result;
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
