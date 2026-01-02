// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QQuickTextDocument>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <qquickitem.h>

#include "chattextitemhelper.h"

class ChatTextItemHelperTestHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QML text Item the TextItemHelper is handling.
     */
    Q_PROPERTY(ChatTextItemHelper *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

public:
    explicit ChatTextItemHelperTestHelper(QObject *parent = nullptr)
        : QObject(parent)
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
        Q_EMIT textItemChanged();
    }

    Q_INVOKABLE void setFixedChars(const QString &startChars, const QString &endChars)
    {
        m_textItem->setFixedChars(startChars, endChars);
    }

    Q_INVOKABLE bool compareDocuments(QQuickTextDocument *document)
    {
        if (!m_textItem) {
            return false;
        }
        return document->textDocument() == m_textItem->document();
    }

    Q_INVOKABLE int lineCount()
    {
        if (!m_textItem) {
            return -1;
        }
        return m_textItem->lineCount();
    }

    Q_INVOKABLE QString firstBlockText()
    {
        if (!m_textItem) {
            return {};
        }
        return m_textItem->takeFirstBlock().toPlainText();
    }

    Q_INVOKABLE bool checkFragments(const QString &before, const QString &mid, const QString &after)
    {
        if (!m_textItem) {
            return false;
        }

        bool hasBefore = false;
        QTextDocumentFragment midFragment;
        std::optional<QTextDocumentFragment> afterFragment = std::nullopt;
        m_textItem->fillFragments(hasBefore, midFragment, afterFragment);

        return hasBefore && m_textItem->document()->toPlainText() == before && midFragment.toPlainText() == mid && after.isEmpty()
            ? !afterFragment
            : afterFragment->toPlainText() == after;
    }

    Q_INVOKABLE void insertFragment(const QString &text, ChatTextItemHelper::InsertPosition position = ChatTextItemHelper::Cursor, bool keepPosition = false)
    {
        if (!m_textItem) {
            return;
        }
        const auto fragment = QTextDocumentFragment::fromPlainText(text);
        m_textItem->insertFragment(fragment, position, keepPosition);
    }

    Q_INVOKABLE bool compareCursor(int cursorPosition, int selectionStart, int selectionEnd)
    {
        if (!m_textItem) {
            return false;
        }
        const auto cursor = m_textItem->textCursor();
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
        if (!m_textItem) {
            return -1;
        }
        return *m_textItem->cursorPosition();
    }

    Q_INVOKABLE void setCursorPosition(int pos)
    {
        if (!m_textItem) {
            return;
        }
        m_textItem->setCursorPosition(pos);
    }

    Q_INVOKABLE void setCursorVisible(bool visible)
    {
        if (!m_textItem) {
            return;
        }
        m_textItem->setCursorVisible(visible);
    }

    Q_INVOKABLE void setCursorFromTextItem(QQuickItem *item, bool infront, int cursorPos)
    {
        if (!m_textItem) {
            return;
        }
        const auto textItem = new ChatTextItemHelper();
        textItem->setTextItem(item);
        textItem->setCursorPosition(cursorPos);
        m_textItem->setCursorFromTextItem(textItem, infront);
    }

    Q_INVOKABLE void mergeFormatOnCursor(RichFormat::Format format)
    {
        if (!m_textItem) {
            return;
        }
        m_textItem->mergeFormatOnCursor(format);
    }

    Q_INVOKABLE bool checkFormatsAtCursor(QList<RichFormat::Format> formats)
    {
        const auto cursor = m_textItem->textCursor();
        if (cursor.isNull()) {
            return false;
        }
        return RichFormat::formatsAtCursor(cursor) == formats;
    }

    Q_INVOKABLE bool canIndentListMoreAtCursor() const
    {
        if (!m_textItem) {
            return false;
        }
        return m_textItem->canIndentListMoreAtCursor();
    }
    Q_INVOKABLE bool canIndentListLessAtCursor() const
    {
        if (!m_textItem) {
            return false;
        }
        return m_textItem->canIndentListLessAtCursor();
    }
    Q_INVOKABLE void indentListMoreAtCursor()
    {
        if (!m_textItem) {
            return;
        }
        m_textItem->indentListMoreAtCursor();
    }
    Q_INVOKABLE void indentListLessAtCursor()
    {
        if (!m_textItem) {
            return;
        }
        m_textItem->indentListLessAtCursor();
    }

    Q_INVOKABLE void forceActiveFocus() const
    {
        if (!m_textItem) {
            return;
        }
        m_textItem->forceActiveFocus();
    }

    Q_INVOKABLE QString markdownText() const
    {
        if (!m_textItem) {
            return {};
        }
        return m_textItem->markdownText();
    }

Q_SIGNALS:
    void textItemChanged();

private:
    QPointer<ChatTextItemHelper> m_textItem;
};
