// Copyright (c) 2020 Christian Mollekopf <mollekopf@kolabsystems.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QTextDocument>
#include <QSyntaxHighlighter>
#include <Sonnet/Speller>
#include <Sonnet/GuessLanguage>

class SpellcheckHighlighter: public QSyntaxHighlighter
{
public:
    SpellcheckHighlighter(QTextDocument *parent);

protected:
    void highlightBlock(const QString &text) override;

private:
    void autodetectLanguage(const QString &sentence);
    QTextCharFormat mErrorFormat;
    QTextCharFormat mQuoteFormat;
    QScopedPointer<Sonnet::Speller> mSpellchecker;
    QScopedPointer<Sonnet::GuessLanguage> mLanguageGuesser;
};

