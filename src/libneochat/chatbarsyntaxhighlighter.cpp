// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "chatbarsyntaxhighlighter.h"

#include "chattextitemhelper.h"

ChatBarSyntaxHighlighter::ChatBarSyntaxHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    m_theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
    connect(m_theme, &Kirigami::Platform::PlatformTheme::colorsChanged, this, [this]() {
        m_errorFormat.setForeground(m_theme->negativeTextColor());
    });

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
        for (const auto &error : std::as_const(m_errors)) {
            setFormat(error.first, error.second.size(), m_errorFormat);
        }
    }
}
