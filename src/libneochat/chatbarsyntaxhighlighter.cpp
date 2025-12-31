// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "chatbarsyntaxhighlighter.h"

#include "chatbarcache.h"
#include "chattextitemhelper.h"
#include "enums/chatbartype.h"

ChatBarSyntaxHighlighter::ChatBarSyntaxHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    m_theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
    connect(m_theme, &Kirigami::Platform::PlatformTheme::colorsChanged, this, [this]() {
        m_mentionFormat.setForeground(m_theme->linkColor());
        m_errorFormat.setForeground(m_theme->negativeTextColor());
    });

    m_mentionFormat.setFontWeight(QFont::Bold);
    m_mentionFormat.setForeground(m_theme->linkColor());

    m_errorFormat.setForeground(m_theme->negativeTextColor());
    m_errorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

    connect(m_checker, &Sonnet::BackgroundChecker::misspelling, this, [this](const QString &word, int start) {
        m_errors += {start, word};
        m_checker->continueChecking();
    });
    connect(m_checker, &Sonnet::BackgroundChecker::done, this, [this]() {
        m_rehighlightTimer.start();
    });
    m_rehighlightTimer.setInterval(100);
    m_rehighlightTimer.setSingleShot(true);
    m_rehighlightTimer.callOnTimeout(this, &QSyntaxHighlighter::rehighlight);
}

void ChatBarSyntaxHighlighter::highlightBlock(const QString &text)
{
    if (m_settings.checkerEnabledByDefault()) {
        if (text != m_previousText) {
            m_previousText = text;
            m_checker->stop();
            m_errors.clear();
            m_checker->setText(text);
        }
        for (const auto &error : m_errors) {
            setFormat(error.first, error.second.size(), m_errorFormat);
        }
    }

    if (!room || type == ChatBarType::None) {
        return;
    }
    auto mentions = room->cacheForType(type)->mentions();
    mentions->erase(std::remove_if(mentions->begin(),
                                   mentions->end(),
                                   [this](auto &mention) {
                                       if (document()->toPlainText().isEmpty()) {
                                           return false;
                                       }

                                       if (mention.cursor.position() == 0 && mention.cursor.anchor() == 0) {
                                           return true;
                                       }

                                       if (mention.cursor.position() - mention.cursor.anchor() != mention.text.size()) {
                                           mention.cursor.setPosition(mention.start);
                                           mention.cursor.setPosition(mention.cursor.anchor() + mention.text.size(), QTextCursor::KeepAnchor);
                                       }

                                       if (mention.cursor.selectedText() != mention.text) {
                                           return true;
                                       }
                                       if (currentBlock() == mention.cursor.block()) {
                                           mention.start = mention.cursor.anchor();
                                           mention.position = mention.cursor.position();
                                           setFormat(mention.cursor.selectionStart(), mention.cursor.selectedText().size(), m_mentionFormat);
                                       }
                                       return false;
                                   }),
                    mentions->end());
}
