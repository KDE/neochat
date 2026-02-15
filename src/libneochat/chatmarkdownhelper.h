// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include "enums/richformat.h"

class QQuickItem;
class QTextDocument;

class ChatTextItemHelper;

class ChatMarkdownHelper : public QObject
{
    Q_OBJECT

public:
    explicit ChatMarkdownHelper(QObject *parent = nullptr);

    ChatTextItemHelper *textItem() const;
    void setTextItem(ChatTextItemHelper *textItem);

Q_SIGNALS:
    void textItemChanged();

    /**
     * @brief There is an unhandled block format request.
     *
     * i.e. the markdown for as new block (e.g. code or quote) has been typed which
     * ChatMarkdownHelper cannot resolve.
     */
    void unhandledBlockFormat(RichFormat::Format format);

private:
    QPointer<ChatTextItemHelper> m_textItem;

    int m_startPos = 0;
    int m_endPos = 0;
    void updateStart();

    void checkMarkdown(int position, int charsRemoved, int charsAdded);
    void updatePosition(int position);
    void checkMarkdownForward();
    void complete();
};
