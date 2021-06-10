// SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
// SPDX-FileCopyrightText: 2020 Christian Mollekopf <mollekopf@kolabsystems.com>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QQuickTextDocument>
#include <QSyntaxHighlighter>
#include <QTextDocument>
#ifndef Q_OS_ANDROID
#include <Sonnet/GuessLanguage>
#include <Sonnet/Speller>
#endif

class SpellcheckHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument *document READ quickDocument WRITE setQuickDocument NOTIFY documentChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)
    Q_PROPERTY(bool wordIsMisspelled READ wordIsMisspelled NOTIFY wordIsMisspelledChanged)
    Q_PROPERTY(QString wordUnderMouse READ wordUnderMouse NOTIFY wordUnderMouseChanged)

public:
    SpellcheckHighlighter(QObject *parent = nullptr);

    Q_INVOKABLE QStringList suggestions(int position, int max = 5);
    Q_INVOKABLE void ignoreWord(const QString &word);
    Q_INVOKABLE void addWordToDictionary(const QString &word);
    Q_INVOKABLE void replaceWord(const QString &word);

    [[nodiscard]] QQuickTextDocument *quickDocument() const;
    void setQuickDocument(QQuickTextDocument *document);

    [[nodiscard]] int cursorPosition() const;
    void setCursorPosition(int position);

    [[nodiscard]] int selectionStart() const;
    void setSelectionStart(int position);

    [[nodiscard]] int selectionEnd() const;
    void setSelectionEnd(int position);

    [[nodiscard]] bool wordIsMisspelled() const;
    [[nodiscard]] QString wordUnderMouse() const;

protected:
    void highlightBlock(const QString &text) override;

Q_SIGNALS:
    void documentChanged();
    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();
    void wordIsMisspelledChanged();
    void wordUnderMouseChanged();
    void changeCursorPosition(int start, int end);

private:
    [[nodiscard]] QTextCursor textCursor() const;
    [[nodiscard]] QTextDocument *textDocument() const;

    void autodetectLanguage(const QString &sentence);
    QTextCharFormat mErrorFormat;
    QTextCharFormat mQuoteFormat;
#ifndef Q_OS_ANDROID
    QScopedPointer<Sonnet::Speller> mSpellchecker;
    QScopedPointer<Sonnet::GuessLanguage> mLanguageGuesser;
#endif
    QString m_selectedWord;
    QQuickTextDocument *m_document;
    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    int m_autoCompleteBeginPosition = -1;
    int m_autoCompleteEndPosition = -1;
    int m_wordIsMisspelled = false;
};
