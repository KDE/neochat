// Copyright (c) 2020 Christian Mollekopf <mollekopf@kolabsystems.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "spellcheckhighlighter.h"

#include <QDebug>
#include <QTextBoundaryFinder>

QVector<QStringRef> split(QTextBoundaryFinder::BoundaryType boundary, const QString &text, int reasonMask = 0)
{
    QVector<QStringRef> parts;
    QTextBoundaryFinder boundaryFinder(boundary, text);

    while (boundaryFinder.position() < text.length()) {
        const int start = boundaryFinder.position();

        //Advance until we find a break that matches the mask or are at the end
        for (;;) {
            if (boundaryFinder.toNextBoundary() == -1) {
                boundaryFinder.toEnd();
                break;
            }
            if (!reasonMask || boundaryFinder.boundaryReasons() & reasonMask) {
                break;
            }
        }

        const auto length = boundaryFinder.position() - start;

        if (length < 1) {
            continue;
        }
        parts << QStringRef{&text, start, length};
    }
    return parts;
}


SpellcheckHighlighter::SpellcheckHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent),
    mSpellchecker{new Sonnet::Speller()},
    mLanguageGuesser{new Sonnet::GuessLanguage()}
{
    //Danger red from our color scheme
    mErrorFormat.setForeground(QColor{"#ed1515"});
    mErrorFormat.setUnderlineColor(QColor{"#ed1515"});
    mErrorFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    mQuoteFormat.setForeground(QColor{"#7f8c8d"});


    if (!mSpellchecker->isValid()) {
        qWarning() << "Spellchecker is invalid";
    }
    qDebug() << "Available dictionaries: " << mSpellchecker->availableDictionaries();
}

void SpellcheckHighlighter::autodetectLanguage(const QString &sentence)
{
    const auto lang = mLanguageGuesser->identify(sentence, mSpellchecker->availableLanguages());
    if (lang.isEmpty()) {
        return;
    }
    mSpellchecker->setLanguage(lang);
}

static bool isSpellcheckable(const QStringRef &token)
{
    if (token.isNull() || token.isEmpty()) {
        return false;
    }
    if (!token.at(0).isLetter() || token.at(0).isUpper() || token.startsWith(QStringLiteral("http"))) {
        return false;
    }
    //TODO ignore urls and uppercase?
    return true;
}

void SpellcheckHighlighter::highlightBlock(const QString &text)
{
    //Avoid spellchecking quotes
    if (text.isEmpty() || text.at(0) == QChar{'>'}) {
        setFormat(0, text.length(), mQuoteFormat);
        return;
    }
    for (const auto &sentenceRef : split(QTextBoundaryFinder::Sentence, text)) {
        //Avoid spellchecking quotes
        if (sentenceRef.isEmpty() || sentenceRef.at(0) == QChar{'>'}) {
            continue;
        }

        const auto sentence = QString::fromRawData(sentenceRef.data(), sentenceRef.length());

        autodetectLanguage(sentence);

        const int offset = sentenceRef.position();
        for (const auto &wordRef : split(QTextBoundaryFinder::Word, sentence)) {
            //Avoid spellchecking words in progress
            //FIXME this will also prevent spellchecking a single word on a line.
            if (offset + wordRef.position() + wordRef.length() >= text.length()) {
                continue;
            }
            if (isSpellcheckable(wordRef)) {
                const auto word = QString::fromRawData(wordRef.data(), wordRef.length());
                const auto format = mSpellchecker->isMisspelled(word) ? mErrorFormat : QTextCharFormat{};
                setFormat(offset + wordRef.position(), wordRef.length(), format);
            }
        }
    }
}
