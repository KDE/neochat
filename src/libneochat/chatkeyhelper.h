// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

class QTextCursor;

class ChatTextItemHelper;

/**
 * @class ChatKeyHelper
 *
 * A class to handle some key presses on behalf of a ChatTextItemHelper.
 *
 * This is used to manage complex rich text interactions.
 *
 * @sa ChatTextItemHelper
 */
class ChatKeyHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ChatKeyHelper(QObject *parent = nullptr);

    /**
     * @brief The ChatTextItemHelper that ChatKeyHelper is handling key presses for.
     *
     * @sa ChatTextItemHelper
     */
    QPointer<ChatTextItemHelper> textItem;

    /**
     * @brief handle the given key and modifiers.
     */
    Q_INVOKABLE bool handleKey(Qt::Key key, Qt::KeyboardModifiers modifiers);

    /**
     * @brief Whether the enter/return should send message.
     *
     * If false, return/enter adds a new line.
     *
     * shift + return/enter does the opposite to return/enter.
     */
    bool sendMessageWithEnter = true;

Q_SIGNALS:
    /**
     * @brief There is an unhandled up key press.
     *
     * Current trigger conditions:
     *  - Up is pressed on the first line of the first block of the text item.
     *  - Return clicked when a completion has been started.
     */
    void unhandledUp(bool isCompleting);

    /**
     * @brief There is an unhandled down key press.
     *
     * Current trigger conditions:
     *  - Down is pressed on the last line of the last block of the text item.
     *  - Return clicked when a completion has been started.
     */
    void unhandledDown(bool isCompleting);

    /**
     * @brief There is an unhandled tab key press.
     *
     * Current trigger conditions:
     *  - Tab clicked when a completion has been started.
     */
    void unhandledTab(bool isCompleting);

    /**
     * @brief There is an unhandled delete key press.
     *
     * Current trigger conditions:
     *  - Delete is pressed at the end of the last line of the last block of the
     *    text item.
     */
    void unhandledDelete();

    /**
     * @brief There is an unhandled backspace key press.
     *
     * Current trigger conditions:
     *  - Backspace is pressed at the beginning of the first line of the first
     *    block of the text item.
     */
    void unhandledBackspace();

    /**
     * @brief There is an unhandled return key press.
     *
     * Current trigger conditions:
     *  - Return clicked when a completion has been started.
     */
    void unhandledReturn(bool isCompleting);

    /**
     * @brief The completion dialog should be closed if open.
     *
     * Current trigger conditions:
     *  - Return clicked when a completion has been started.
     */
    void closeCompletion();

    /**
     * @brief An image has been pasted.
     */
    void imagePasted(const QString &filePath);

private:
    bool vKey(Qt::KeyboardModifiers modifiers);

    bool up(Qt::KeyboardModifiers modifiers);

    bool down();

    bool left();

    bool right();

    bool tab();

    bool deleteChar();

    bool backspace();

    bool insertReturn(Qt::KeyboardModifiers modifiers);

    bool cancel();

    bool pasteImage();

    bool selectLink(QTextCursor &cursor, bool back);
};
