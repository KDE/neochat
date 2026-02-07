// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>

class QTextDocument;

class StyleDelegateHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QML text Item the StyleDelegateHelper is handling.
     */
    Q_PROPERTY(QQuickItem *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    /**
     * @brief Whether the current block is a Quote block.
     */
    Q_PROPERTY(bool inQuote READ inQuote WRITE setInQuote NOTIFY inQuoteChanged)

public:
    explicit StyleDelegateHelper(QObject *parent = nullptr);

    QQuickItem *textItem() const;
    void setTextItem(QQuickItem *textItem);

    bool inQuote() const;
    void setInQuote(bool inQuote);

Q_SIGNALS:
    void textItemChanged();
    void inQuoteChanged();

private:
    QPointer<QQuickItem> m_textItem;
    QTextDocument *document() const;

    bool m_inQuote = false;

private Q_SLOTS:
    void formatDocument();
};
