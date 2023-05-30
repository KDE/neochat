// SPDX-FileCopyrightText: Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QGuiApplication>
#include <QPalette>
#include <QQuickTextDocument>
#include <QTextBoundaryFinder>
#include <QTextCharFormat>
#include <QTextCursor>

#include <KLocalizedString>

#include <unicode/uchar.h>
#include <unicode/urename.h>

#include "emojifixer.h"

bool isEmoji(const QString &text)
{
    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);
    int from = 0;
    while (finder.toNextBoundary() != -1) {
        auto to = finder.position();
        if (text[from].isSpace()) {
            from = to;
            continue;
        }

        auto first = text.mid(from, to - from).toUcs4()[0];
        if (!u_hasBinaryProperty(first, UCHAR_EMOJI_PRESENTATION)) {
            return false;
        }
        from = to;
    }
    return true;
}

void EmojiFixer::addTextDocument(QQuickTextDocument *document)
{
    if (!document) {
        return;
    }
    fix(document->textDocument());
}

void EmojiFixer::fix(QTextDocument *document)
{
    disconnect(document, nullptr, this, nullptr);
    QTextCursor curs(document);
    QTextCharFormat format;
    auto font = QGuiApplication::font();
    font.setFamily("emoji");
    format.setFont(font);

    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, document->toRawText());

    int from = 0;
    while (finder.toNextBoundary() != -1) {
        auto to = finder.position();

        auto first = document->toRawText().mid(from, to - from).toUcs4()[0];
        if (u_hasBinaryProperty(first, UCHAR_EMOJI_PRESENTATION)) {
            curs.setPosition(from, QTextCursor::MoveAnchor);
            curs.setPosition(to, QTextCursor::KeepAnchor);
            curs.setCharFormat(format);
        }
        from = to;
    }
    connect(document, &QTextDocument::contentsChanged, this, [this, document]() {
        fix(document);
    });
}
