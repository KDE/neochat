// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QQuickTextDocument>
#include <QTextCursor>

#include "chattextitemhelper.h"

class ChatTextItemHelperTestWrapper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QML text Item the TextItemHelper is handling.
     */
    Q_PROPERTY(QQuickItem *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

public:
    explicit ChatTextItemHelperTestWrapper(QObject *parent = nullptr)
        : QObject(parent)
        , m_textItemWrapper(new ChatTextItemHelper(this))
    {
        Q_ASSERT(m_textItemWrapper);
        connect(m_textItemWrapper, &ChatTextItemHelper::textItemChanged, this, &ChatTextItemHelperTestWrapper::textItemChanged);
        connect(m_textItemWrapper, &ChatTextItemHelper::contentsChange, this, &ChatTextItemHelperTestWrapper::contentsChange);
        connect(m_textItemWrapper, &ChatTextItemHelper::contentsChanged, this, &ChatTextItemHelperTestWrapper::contentsChanged);
        connect(m_textItemWrapper, &ChatTextItemHelper::cursorPositionChanged, this, &ChatTextItemHelperTestWrapper::cursorPositionChanged);
    }

    QQuickItem *textItem() const
    {
        return m_textItemWrapper->textItem();
    }
    void setTextItem(QQuickItem *textItem)
    {
        m_textItemWrapper->setTextItem(textItem);
    }

    Q_INVOKABLE bool compareDocuments(QQuickTextDocument *document)
    {
        return document->textDocument() == m_textItemWrapper->document();
    }

    Q_INVOKABLE bool compareCursor(int cursorPosition, int selectionStart, int selectionEnd)
    {
        const auto cursor = m_textItemWrapper->textCursor();
        if (cursor.isNull()) {
            return false;
        }
        const auto posSame = cursor.position() == cursorPosition;
        const auto startSame = cursor.selectionStart() == selectionStart;
        const auto endSame = cursor.selectionEnd() == selectionEnd;
        return posSame && startSame && endSame;
    }

    Q_INVOKABLE int cursorPosition() const
    {
        return m_textItemWrapper->cursorPosition();
    }

    Q_INVOKABLE void setCursorPosition(int pos)
    {
        m_textItemWrapper->setCursorPosition(pos);
    }

    Q_INVOKABLE void setCursorVisible(bool visible)
    {
        m_textItemWrapper->setCursorVisible(visible);
    }

    Q_INVOKABLE void forceActiveFocus() const
    {
        m_textItemWrapper->forceActiveFocus();
    }

Q_SIGNALS:
    void textItemChanged();
    void contentsChange(int position, int charsRemoved, int charsAdded);
    void contentsChanged();
    void cursorPositionChanged();

private:
    QPointer<ChatTextItemHelper> m_textItemWrapper;
};
