// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

#include "chattextitemhelper.h"

class ChatButtonHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The text item that the helper is interfacing with.
     *
     * This is a QQuickItem that is a TextEdit (or inherited from) wrapped in a ChatTextItemHelper
     * to provide easy access to properties and basic QTextDocument manipulation.
     *
     * @sa TextEdit, QTextDocument, ChatTextItemHelper
     */
    Q_PROPERTY(ChatTextItemHelper *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    /**
     * @brief Whether the text format at the current cursor is bold.
     */
    Q_PROPERTY(bool bold READ bold NOTIFY textFormatChanged)

    /**
     * @brief Whether the text format at the current cursor is italic.
     */
    Q_PROPERTY(bool italic READ italic NOTIFY textFormatChanged)

    /**
     * @brief Whether the text format at the current cursor is underlined.
     */
    Q_PROPERTY(bool underline READ underline NOTIFY textFormatChanged)

    /**
     * @brief Whether the text format at the current cursor is struckthrough.
     */
    Q_PROPERTY(bool strikethrough READ strikethrough NOTIFY textFormatChanged)

    /**
     * @brief Whether the format at the current cursor includes RichFormat::UnorderedList.
     */
    Q_PROPERTY(bool unorderedList READ unorderedList NOTIFY listChanged)

    /**
     * @brief Whether the format at the current cursor includes RichFormat::OrderedList.
     */
    Q_PROPERTY(bool orderedList READ orderedList NOTIFY listChanged)

    /**
     * @brief The current style at the cursor.
     */
    Q_PROPERTY(RichFormat::Format currentStyle READ currentStyle NOTIFY styleChanged)

    /**
     * @brief Whether the list at the current cursor can be indented one level more.
     */
    Q_PROPERTY(bool canIndentListMore READ canIndentListMore NOTIFY listChanged)

    /**
     * @brief Whether the list at the current cursor can be indented one level less.
     */
    Q_PROPERTY(bool canIndentListLess READ canIndentListLess NOTIFY listChanged)

    /**
     * @brief The link url at the current cursor position.
     */
    Q_PROPERTY(QString currentLinkUrl READ currentLinkUrl NOTIFY linkChanged)

    /**
     * @brief The link url at the current cursor position.
     */
    Q_PROPERTY(QString currentLinkText READ currentLinkText NOTIFY linkChanged)

public:
    explicit ChatButtonHelper(QObject *parent = nullptr);

    ChatTextItemHelper *textItem() const;
    void setTextItem(ChatTextItemHelper *textItem);

    bool bold() const;
    bool italic() const;
    bool underline() const;
    bool strikethrough() const;
    bool unorderedList() const;
    bool orderedList() const;
    RichFormat::Format currentStyle() const;

    /**
     * @brief Apply the given format at the current cursor position.
     */
    Q_INVOKABLE void setFormat(RichFormat::Format format);

    bool canIndentListMore() const;
    bool canIndentListLess() const;

    /**
     * @brief Indent the list at the current cursor one level more.
     */
    Q_INVOKABLE void indentListMore();

    /**
     * @brief Indent the list at the current cursor one level less.
     */
    Q_INVOKABLE void indentListLess();

    /**
     * @brief Insert text at the current cursor position.
     */
    Q_INVOKABLE void insertText(const QString &text);

    QString currentLinkUrl() const;
    QString currentLinkText() const;

    /**
     * @brief Update the link at the current cursor position.
     *
     * This will replace any selected text of the word next to the cursor with the
     * given text and will link to the given url.
     */
    Q_INVOKABLE void updateLink(const QString &linkUrl, const QString &linkText);

Q_SIGNALS:
    void textItemChanged();
    void formatChanged();
    void textFormatChanged();
    void styleChanged();
    void listChanged();
    void linkChanged();

private:
    QPointer<ChatTextItemHelper> m_textItem;

    void selectLinkText(QTextCursor &cursor) const;
};
