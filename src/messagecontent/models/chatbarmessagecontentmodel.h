// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "block.h"
#include "blockcache.h"
#include "chatbarcache.h"
#include "chatkeyhelper.h"
#include "chatmarkdownhelper.h"
#include "chattextitemhelper.h"
#include "enums/blocktype.h"
#include "enums/richformat.h"
#include "models/messagecontentmodel.h"

/**
 * @class ChatBarMessageContentModel
 *
 * Inherited from MessageContentModel this visualises the content of a Quotient::RoomMessageEvent.
 */
class ChatBarMessageContentModel : public MessageContentModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QQuickTextDocument that is being handled.
     */
    Q_PROPERTY(ChatBarType::Type type READ type WRITE setType NOTIFY typeChanged)

    /**
     * @brief The row of the model component that currently has focus.
     */
    Q_PROPERTY(int focusRow READ focusRow WRITE setFocusRow NOTIFY focusRowChanged)

    /**
     * @brief The Blocks::Type of the focussed row.
     */
    Q_PROPERTY(Blocks::Type focusType READ focusType NOTIFY focusRowChanged)

    /**
     * @brief The text item that the helper is interfacing with.
     *
     * This is a QQuickItem that is a TextEdit (or inherited from) wrapped in a ChatTextItemHelper
     * to provide easy access to properties and basic QTextDocument manipulation.
     *
     * @sa TextEdit, QTextDocument, ChatTextItemHelper
     */
    Q_PROPERTY(ChatKeyHelper *keyHelper READ keyHelper CONSTANT)

    /**
     * @brief The text item that the helper is interfacing with.
     *
     * This is a QQuickItem that is a TextEdit (or inherited from) wrapped in a ChatTextItemHelper
     * to provide easy access to properties and basic QTextDocument manipulation.
     *
     * @sa TextEdit, QTextDocument, ChatTextItemHelper
     */
    Q_PROPERTY(ChatTextItemHelper *focusedTextItem READ focusedTextItem NOTIFY focusRowChanged)

    /**
     * @brief Whether there is any rich formatting in any of the model components.
     *
     * If true the contents of the model will change if an attachment is added.
     */
    Q_PROPERTY(bool hasRichFormatting READ hasRichFormatting NOTIFY hasRichFormattingChanged)

    /**
     * @brief Whether the model has an attachment..
     */
    Q_PROPERTY(bool hasAttachment READ hasAttachment NOTIFY hasAttachmentChanged)

    /**
     * @brief The UserListModel to be used for room completions.
     */
    Q_PROPERTY(bool sendMessageWithEnter READ sendMessageWithEnter WRITE setSendMessageWithEnter NOTIFY sendMessageWithEnterChanged)

    /**
     * @brief Whether the model has any content, ideal for checking if there is anything to send.
     */
    Q_PROPERTY(bool hasAnyContent READ hasAnyContent NOTIFY hasAnyContentChanged)

    /**
     * @brief Whether to send typing notifications to the server when the content changes.
     */
    Q_PROPERTY(bool sendTypingNotifications READ sendTypingNotifications WRITE setSendTypingNotifications)

public:
    explicit ChatBarMessageContentModel(QObject *parent = nullptr);

    ChatBarType::Type type() const;
    void setType(ChatBarType::Type type);
    ChatKeyHelper *keyHelper() const;
    int focusRow() const;
    Blocks::Type focusType() const;
    Q_INVOKABLE void setFocusRow(int focusRow, bool mouse = false);
    Q_INVOKABLE void refocusCurrentComponent() const;
    ChatTextItemHelper *focusedTextItem() const;

    Q_INVOKABLE void insertStyleAtCursor(RichFormat::Format style);

    bool hasRichFormatting() const;
    bool hasAttachment() const;
    Q_INVOKABLE void addAttachment(const QUrl &path);
    Q_INVOKABLE void drop(QList<QUrl> urls, const QString &transferPortal);

    Q_INVOKABLE void removeComponent(int row, bool removeLast = false);

    Q_INVOKABLE void removeAttachment();

    bool sendMessageWithEnter() const;
    void setSendMessageWithEnter(bool sendMessageWithEnter);

    void setSendTypingNotifications(bool sendTypingNotifications);
    [[nodiscard]] bool sendTypingNotifications() const;

    Q_INVOKABLE void resetModel();

    Q_INVOKABLE void postMessage();

    bool hasAnyContent() const;

    static bool richTextActive;

Q_SIGNALS:
    void typeChanged(ChatBarType::Type oldType, ChatBarType::Type newType);
    void focusRowChanged();
    void hasRichFormattingChanged();
    void hasAttachmentChanged();
    void sendMessageWithEnterChanged();
    void hasAnyContentChanged();

private:
    ChatBarType::Type m_type = ChatBarType::None;
    std::optional<ChatBarType::Type> unhandledTypeChange = std::nullopt;
    void connectCache(ChatBarCache *oldCache = nullptr);

    void initializeModel(const QString &initialText = {});
    void initializeFromCache();
    void initializeEdit();

    std::optional<QString> getReplyEventId() override;

    void setFocusIndex(const QModelIndex &index, bool mouse = false);
    void focusCurrentComponent(const QModelIndex &previousIndex, bool down);
    void emitFocusChangeSignals();

    void connectTextItem(ChatTextItemHelper *chattextitemhelper);
    ChatTextItemHelper *textItemForComponent(Blocks::Block *component) const;
    ChatTextItemHelper *textItemForIndex(const QModelIndex &index) const;
    QModelIndex indexForTextItem(ChatTextItemHelper *textItem) const;

    QPointer<ChatMarkdownHelper> m_markdownHelper;
    QPointer<ChatKeyHelper> m_keyHelper;
    void connectKeyHelper();

    Blocks::BlockPtr blockForFile(const QUrl &path);

    void insertBlockAtCursor(Blocks::Type type);
    Blocks::BlockPtr makeEmptyTextBlock(Blocks::Type type);
    Blocks::BlockPtrsIt insertComponent(int row, Blocks::BlockPtr block);
    Blocks::BlockPtrsIt removeComponent(Blocks::BlockPtrsIt it);
    void removeComponent(ChatTextItemHelper *textItem);

    void insertComponentFromCache(Blocks::CacheItem *item);

    void handleBlockTransition(bool up);

    void updateCache() const;

    bool m_sendMessageWithEnter = true;
    bool m_sendTypingNotifications = false;

    void clearModel();

    QTimer *m_typingTimer;
    void handleTyping();
};
