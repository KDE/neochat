// SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
// SPDX-FileCopyrightText: 2020 Christian Mollekopf <mollekopf@kolabsystems.com>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "spellcheckhighlighter.h"

#include <QHash>
#include <QTextBoundaryFinder>

// Cache of previously-determined languages (when using AutoDetectLanguage)
// There is one such cache per block (paragraph)
class LanguageCache : public QTextBlockUserData
{
public:
    // Key: QPair<start, length>
    // Value: language name
    QMap<QPair<int, int>, QString> languages;

    // Remove all cached language information after @p pos
    void invalidate(int pos)
    {
        QMutableMapIterator<QPair<int, int>, QString> it(languages);
        it.toBack();
        while (it.hasPrevious()) {
            it.previous();
            if (it.key().first + it.key().second >= pos) {
                it.remove();
            } else {
                break;
            }
        }
    }

    QString languageAtPos(int pos) const
    {
        // The data structure isn't really great for such lookups...
        QMapIterator<QPair<int, int>, QString> it(languages);
        while (it.hasNext()) {
            it.next();
            if (it.key().first <= pos && it.key().first + it.key().second >= pos) {
                return it.value();
            }
        }
        return QString();
    }
};

QVector<QStringRef> split(QTextBoundaryFinder::BoundaryType boundary, const QString &text, int reasonMask = 0)
{
    QVector<QStringRef> parts;
    QTextBoundaryFinder boundaryFinder(boundary, text);

    while (boundaryFinder.position() < text.length()) {
        const int start = boundaryFinder.position();

        // Advance until we find a break that matches the mask or are at the end
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

SpellcheckHighlighter::SpellcheckHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
#ifndef Q_OS_ANDROID
    , mSpellchecker{new Sonnet::Speller()}
    , mLanguageGuesser{new Sonnet::GuessLanguage()}
#endif
    , m_document(nullptr)
    , m_cursorPosition(-1)
{
    // Danger red from our color scheme
    mErrorFormat.setForeground(QColor{"#ed1515"});
    mErrorFormat.setUnderlineColor(QColor{"#ed1515"});
    mErrorFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    mQuoteFormat.setForeground(QColor{"#7f8c8d"});
#ifndef Q_OS_ANDROID
    if (!mSpellchecker->isValid()) {
        qWarning() << "Spellchecker is invalid";
    }
#endif
}

void SpellcheckHighlighter::autodetectLanguage(const QString &sentence)
{
#ifndef Q_OS_ANDROID
    const auto lang = mLanguageGuesser->identify(sentence, mSpellchecker->availableLanguages());
    if (lang.isEmpty()) {
        return;
    }
    mSpellchecker->setLanguage(lang);
#endif
}

static bool isSpellcheckable(const QStringRef &token)
{
    if (token.isNull() || token.isEmpty()) {
        return false;
    }
    if (!token.at(0).isLetter() || token.at(0).isUpper() || token.startsWith(QStringLiteral("http"))) {
        return false;
    }
    // part of a slash command
    if (token.contains("rainbowme") || token.contains("lenny")) {
        return false;
    }
    // TODO ignore urls and uppercase?
    return true;
}

void SpellcheckHighlighter::highlightBlock(const QString &text)
{
    // Avoid spellchecking quotes
    if (text.isEmpty() || text.at(0) == QLatin1Char('>')) {
        setFormat(0, text.length(), mQuoteFormat);
        return;
    }
    // Don't spell check certain commands
    if (text.startsWith("/join") || text.startsWith("/part") || text.startsWith("/invite")) {
        setFormat(0, text.length(), QTextCharFormat{});
        return;
    }
#ifndef Q_OS_ANDROID
    for (const auto &sentenceRef : split(QTextBoundaryFinder::Sentence, text)) {
        // Avoid spellchecking quotes
        if (sentenceRef.isEmpty() || sentenceRef.at(0) == QLatin1Char('>')) {
            continue;
        }

        const auto sentence = QString::fromRawData(sentenceRef.data(), sentenceRef.length());

        autodetectLanguage(sentence);

        const int offset = sentenceRef.position();
        for (const auto &wordRef : split(QTextBoundaryFinder::Word, sentence)) {
            // Avoid spellchecking words in progress
            // FIXME this will also prevent spellchecking a single word on a line.
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
#endif
}

QStringList SpellcheckHighlighter::suggestions(int mousePosition, int max)
{
#ifndef Q_OS_ANDROID
    QTextCursor cursor = textCursor();

    QTextCursor cursorAtMouse(textDocument());
    cursorAtMouse.setPosition(mousePosition);

    // Check if the user clicked a selected word
    /* clang-format off */
    const bool selectedWordClicked = cursor.hasSelection()
                                     && mousePosition >= cursor.selectionStart()
                                     && mousePosition <= cursor.selectionEnd();
    /* clang-format on */

    // Get the word under the (mouse-)cursor and see if it is misspelled.
    // Don't include apostrophes at the start/end of the word in the selection.
    QTextCursor wordSelectCursor(cursorAtMouse);
    wordSelectCursor.clearSelection();
    wordSelectCursor.select(QTextCursor::WordUnderCursor);
    m_selectedWord = wordSelectCursor.selectedText();

    // Clear the selection again, we re-select it below (without the apostrophes).
    wordSelectCursor.setPosition(wordSelectCursor.position() - m_selectedWord.size());
    if (m_selectedWord.startsWith(QLatin1Char('\'')) || m_selectedWord.startsWith(QLatin1Char('\"'))) {
        m_selectedWord = m_selectedWord.right(m_selectedWord.size() - 1);
        wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
    }
    if (m_selectedWord.endsWith(QLatin1Char('\'')) || m_selectedWord.endsWith(QLatin1Char('\"'))) {
        m_selectedWord.chop(1);
    }

    wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, m_selectedWord.size());

    int endSelection = wordSelectCursor.selectionEnd();
    Q_EMIT wordUnderMouseChanged();

    bool isMouseCursorInsideWord = true;
    if ((mousePosition < wordSelectCursor.selectionStart() || mousePosition >= wordSelectCursor.selectionEnd()) //
        && (m_selectedWord.length() > 1)) {
        isMouseCursorInsideWord = false;
    }

    wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, m_selectedWord.size());

    m_wordIsMisspelled = isMouseCursorInsideWord && !m_selectedWord.isEmpty() && mSpellchecker->isMisspelled(m_selectedWord);
    Q_EMIT wordIsMisspelledChanged();

    if (!m_wordIsMisspelled || selectedWordClicked) {
        return QStringList{};
    }

    if (!selectedWordClicked) {
        Q_EMIT changeCursorPosition(wordSelectCursor.selectionStart(), endSelection);
    }

    LanguageCache *cache = dynamic_cast<LanguageCache *>(cursor.block().userData());
    if (cache) {
        const QString cachedLanguage = cache->languageAtPos(cursor.positionInBlock());
        if (!cachedLanguage.isEmpty()) {
            mSpellchecker->setLanguage(cachedLanguage);
        }
    }
    QStringList suggestions = mSpellchecker->suggest(m_selectedWord);
    if (max >= 0 && suggestions.count() > max) {
        suggestions = suggestions.mid(0, max);
    }

    return suggestions;
#else 
    return QStringList();
#endif
}

void SpellcheckHighlighter::addWordToDictionary(const QString &word)
{
#ifndef Q_OS_ANDROID
    mSpellchecker->addToPersonal(word);
    rehighlight();
#endif
}

void SpellcheckHighlighter::ignoreWord(const QString &word)
{
#ifndef Q_OS_ANDROID
    mSpellchecker->addToSession(word);
    rehighlight();
#endif
}

void SpellcheckHighlighter::replaceWord(const QString &replacement)
{
#ifndef Q_OS_ANDROID
    textCursor().insertText(replacement);
#endif
}

QQuickTextDocument *SpellcheckHighlighter::quickDocument() const
{
    return m_document;
}

void SpellcheckHighlighter::setQuickDocument(QQuickTextDocument *document)
{
    if (document == m_document) {
        return;
    }

    if (m_document) {
        m_document->textDocument()->disconnect(this);
    }
    m_document = document;
    setDocument(document->textDocument());
    Q_EMIT documentChanged();
}

int SpellcheckHighlighter::cursorPosition() const
{
    return m_cursorPosition;
}

void SpellcheckHighlighter::setCursorPosition(int position)
{
    if (position == m_cursorPosition) {
        return;
    }

    m_cursorPosition = position;
    Q_EMIT cursorPositionChanged();
}

int SpellcheckHighlighter::selectionStart() const
{
    return m_selectionStart;
}

void SpellcheckHighlighter::setSelectionStart(int position)
{
    if (position == m_selectionStart) {
        return;
    }

    m_selectionStart = position;
    Q_EMIT selectionStartChanged();
}

int SpellcheckHighlighter::selectionEnd() const
{
    return m_selectionEnd;
}

void SpellcheckHighlighter::setSelectionEnd(int position)
{
    if (position == m_selectionEnd) {
        return;
    }

    m_selectionEnd = position;
    Q_EMIT selectionEndChanged();
}

QTextCursor SpellcheckHighlighter::textCursor() const
{
    QTextDocument *doc = textDocument();
    if (!doc) {
        return QTextCursor();
    }

    QTextCursor cursor(doc);
    if (m_selectionStart != m_selectionEnd) {
        cursor.setPosition(m_selectionStart);
        cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(m_cursorPosition);
    }
    return cursor;
}

QTextDocument *SpellcheckHighlighter::textDocument() const
{
    if (!m_document) {
        return nullptr;
    }

    return m_document->textDocument();
}

bool SpellcheckHighlighter::wordIsMisspelled() const
{
    return m_wordIsMisspelled;
}

QString SpellcheckHighlighter::wordUnderMouse() const
{
    return m_selectedWord;
}
