// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

#include "chattextitemhelper.h"

class ChatKeyHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ChatKeyHelper(QObject *parent = nullptr);

    ChatTextItemHelper *textItem() const;
    void setTextItem(ChatTextItemHelper *textItem);

    /**
     * @brief Handle up key at current cursor location.
     */
    Q_INVOKABLE void up();

    /**
     * @brief Handle down key at current cursor location.
     */
    Q_INVOKABLE void down();

    /**
     * @brief Handle tab key at current cursor location.
     */
    Q_INVOKABLE void tab();

    /**
     * @brief Handle delete key at current cursor location.
     */
    Q_INVOKABLE void deleteChar();

    /**
     * @brief Handle backspace key at current cursor location.
     */
    Q_INVOKABLE void backspace();

    /**
     * @brief Handle return key at current cursor location.
     */
    Q_INVOKABLE void insertReturn();

Q_SIGNALS:
    void textItemChanged();

    /**
     * @brief There is an unhandled up key press.
     *
     * i.e. up is pressed on the first line of the first block of the text item.
     */
    void unhandledUp();

    /**
     * @brief There is an unhandled down key press.
     *
     * i.e. down is pressed on the last line of the last block of the text item.
     */
    void unhandledDown();

    /**
     * @brief There is an unhandled delete key press.
     *
     * i.e. delete is pressed at the end of the last line of the last block of the
     * text item.
     */
    void unhandledDelete();

    /**
     * @brief There is an unhandled backspace key press.
     *
     * i.e. backspace is pressed at the beginning of the first line of the first
     * block of the text item.
     */
    void unhandledBackspace();

private:
    QPointer<ChatTextItemHelper> m_textItem;
};
