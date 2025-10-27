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
     * @brief The QML text Item the ChatDocumentHandler is handling.
     */
    Q_PROPERTY(QQuickItem *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

public:
    explicit StyleDelegateHelper(QObject *parent = nullptr);

    QQuickItem *textItem() const;
    void setTextItem(QQuickItem *textItem);

Q_SIGNALS:
    void textItemChanged();

private:
    QPointer<QQuickItem> m_textItem;
    QTextDocument *document() const;

    void formatDocument();
};
