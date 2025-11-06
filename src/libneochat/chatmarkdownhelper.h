// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

#include "enums/richformat.h"

class QTextDocument;

class ChatDocumentHandler;

class ChatMarkdownHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ChatMarkdownHelper(ChatDocumentHandler *parent);

    void handleExternalFormatChange();

private:
    enum State {
        None,
        Pre,
        Started,
    };

    QTextDocument *document() const;
    void connectDocument();

    State m_currentState = None;
    int m_startPos = 0;
    int m_endPos = 0;

    QHash<RichFormat::Format, int> m_currentFormats;

    void checkMarkdown(int position, int charsRemoved, int charsAdded);
    void complete();
};
