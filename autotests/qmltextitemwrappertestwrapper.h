// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QQuickTextDocument>
#include <QTextCursor>

#include "qmltextitemwrapper.h"

class QmlTextItemWrapperTestWrapper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QML text Item the ChatDocumentHandler is handling.
     */
    Q_PROPERTY(QQuickItem *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

public:
    explicit QmlTextItemWrapperTestWrapper(QObject *parent = nullptr)
        : QObject(parent)
        , m_textItemWrapper(new QmlTextItemWrapper(this))
    {
        Q_ASSERT(m_textItemWrapper);
        connect(m_textItemWrapper, &QmlTextItemWrapper::textItemChanged, this, &QmlTextItemWrapperTestWrapper::textItemChanged);
        connect(m_textItemWrapper, &QmlTextItemWrapper::textDocumentContentsChange, this, &QmlTextItemWrapperTestWrapper::textDocumentContentsChange);
        connect(m_textItemWrapper, &QmlTextItemWrapper::textDocumentContentsChanged, this, &QmlTextItemWrapperTestWrapper::textDocumentContentsChanged);
        connect(m_textItemWrapper,
                &QmlTextItemWrapper::textDocumentCursorPositionChanged,
                this,
                &QmlTextItemWrapperTestWrapper::textDocumentCursorPositionChanged);
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
    void textDocumentContentsChange(int position, int charsRemoved, int charsAdded);
    void textDocumentContentsChanged();
    void textDocumentCursorPositionChanged();

private:
    QPointer<QmlTextItemWrapper> m_textItemWrapper;
};
