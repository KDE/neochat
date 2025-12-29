// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include "enums/richformat.h"

class QQuickItem;
class QTextDocument;

class QmlTextItemWrapper;

class ChatMarkdownHelper : public QObject
{
    Q_OBJECT

public:
    explicit ChatMarkdownHelper(QObject *parent = nullptr);

    QmlTextItemWrapper *textItem() const;
    void setTextItem(QmlTextItemWrapper *textItem);

    void handleExternalFormatChange();

Q_SIGNALS:
    void textItemChanged();
    void unhandledBlockFormat(RichFormat::Format format);

private:
    enum State {
        None,
        Pre,
        Started,
    };

    QPointer<QmlTextItemWrapper> m_textItem;

    State m_currentState = None;
    int m_startPos = 0;
    int m_endPos = 0;

    QHash<RichFormat::Format, int> m_currentFormats;

    void checkMarkdown(int position, int charsRemoved, int charsAdded);
    void complete();
};
