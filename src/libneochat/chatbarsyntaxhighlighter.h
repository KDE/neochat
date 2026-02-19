// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QQuickTextDocument>
#include <QSyntaxHighlighter>
#include <QTimer>

#include <Kirigami/Platform/PlatformTheme>

#include <Sonnet/BackgroundChecker>
#include <Sonnet/Settings>

#include "chattextitemhelper.h"
#include "neochatroom.h"

class ChatBarSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit ChatBarSyntaxHighlighter(QObject *parent = nullptr);

    QPointer<NeoChatRoom> room;
    ChatBarType::Type type = ChatBarType::None;

    ChatTextItemHelper *textItem() const;
    void setTextItem(ChatTextItemHelper *textItem);

    void highlightBlock(const QString &text) override;

private:
    Kirigami::Platform::PlatformTheme *m_theme = nullptr;
    QTextCharFormat m_errorFormat;

    Sonnet::BackgroundChecker *m_checker = new Sonnet::BackgroundChecker(this);
    Sonnet::Settings m_settings;
    QString m_previousText;

    QList<QPair<int, QString>> m_errors;
    QTimer m_rehighlightTimer;
};
