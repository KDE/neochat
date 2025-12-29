// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QTextCursor>
#include <qtextcursor.h>

#include "chatmarkdownhelper.h"
#include "enums/richformat.h"
#include "qmltextitemwrapper.h"

class ChatMarkdownHelperTestWrapper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QML text Item the ChatDocumentHandler is handling.
     */
    Q_PROPERTY(QQuickItem *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

public:
    explicit ChatMarkdownHelperTestWrapper(QObject *parent = nullptr)
        : QObject(parent)
        , m_chatMarkdownHelper(new ChatMarkdownHelper(this))
        , m_textItem(new QmlTextItemWrapper(this))
    {
        connect(m_chatMarkdownHelper, &ChatMarkdownHelper::textItemChanged, this, &ChatMarkdownHelperTestWrapper::textItemChanged);
        connect(m_chatMarkdownHelper, &ChatMarkdownHelper::unhandledBlockFormat, this, &ChatMarkdownHelperTestWrapper::unhandledBlockFormat);
    }

    QQuickItem *textItem() const
    {
        return m_chatMarkdownHelper->textItem()->textItem();
    }
    void setTextItem(QQuickItem *textItem)
    {
        auto textItemWrapper = new QmlTextItemWrapper(this);
        textItemWrapper->setTextItem(textItem);
        m_chatMarkdownHelper->setTextItem(textItemWrapper);
        m_textItem->setTextItem(textItem);
    }

    Q_INVOKABLE bool checkText(const QString &text)
    {
        const auto doc = m_textItem->document();
        if (!doc) {
            return false;
        }
        qWarning() << text << doc->toPlainText();
        return text == doc->toPlainText();
    }

    Q_INVOKABLE bool checkFormats(QList<RichFormat::Format> formats)
    {
        const auto cursor = m_textItem->textCursor();
        if (cursor.isNull()) {
            return false;
        }
        qWarning() << RichFormat::formatsAtCursor(cursor) << formats;
        return RichFormat::formatsAtCursor(cursor) == formats;
    }

    Q_INVOKABLE void clear()
    {
        auto cursor = m_textItem->textCursor();
        if (cursor.isNull()) {
            return;
        }
        cursor.select(QTextCursor::Document);
        cursor.removeSelectedText();
        cursor.setBlockCharFormat(RichFormat::charFormatForFormat(RichFormat::Paragraph));
        cursor.setBlockFormat(RichFormat::blockFormatForFormat(RichFormat::Paragraph));
    }

Q_SIGNALS:
    void textItemChanged();
    void unhandledBlockFormat(RichFormat::Format format);

private:
    QPointer<ChatMarkdownHelper> m_chatMarkdownHelper;
    QPointer<QmlTextItemWrapper> m_textItem;
};
