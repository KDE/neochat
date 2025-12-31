// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "chatkeyhelper.h"
#include "chatmarkdownhelper.h"
#include "chattextitemhelper.h"
#include "enums/messagecomponenttype.h"
#include "enums/richformat.h"
#include "messagecomponent.h"
#include "models/messagecontentmodel.h"

/**
 * @class ChatBarMessageContentModel
 *
 * Inherited from MessageContentModel this visulaises the content of a Quotient::RoomMessageEvent.
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
     * @brief The MessageComponentType of the focussed row.
     */
    Q_PROPERTY(MessageComponentType::Type focusType READ focusType NOTIFY focusRowChanged)

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

public:
    explicit ChatBarMessageContentModel(QObject *parent = nullptr);

    ChatBarType::Type type() const;
    void setType(ChatBarType::Type type);
    ChatKeyHelper *keyHelper() const;
    int focusRow() const;
    MessageComponentType::Type focusType() const;
    Q_INVOKABLE void setFocusRow(int focusRow, bool mouse = false);
    Q_INVOKABLE void refocusCurrentComponent() const;
    ChatTextItemHelper *focusedTextItem() const;

    Q_INVOKABLE void insertStyleAtCursor(RichFormat::Format style);

    Q_INVOKABLE void insertComponentAtCursor(MessageComponentType::Type type);

    Q_INVOKABLE void addAttachment(const QUrl &path);

    Q_INVOKABLE void removeComponent(int row, bool removeLast = false);

    Q_INVOKABLE void removeAttachment();

    Q_INVOKABLE void postMessage();

Q_SIGNALS:
    void typeChanged();
    void focusRowChanged();

private:
    ChatBarType::Type m_type = ChatBarType::None;

    void initializeModel();

    std::optional<QString> getReplyEventId() override;

    void setFocusIndex(const QModelIndex &index, bool mouse = false);
    void focusCurrentComponent(const QModelIndex &previousIndex, bool down);
    void emitFocusChangeSignals();

    void connectTextItem(ChatTextItemHelper *chattextitemhelper);
    ChatTextItemHelper *textItemForComponent(const MessageComponent &component) const;
    ChatTextItemHelper *textItemForIndex(const QModelIndex &index) const;
    QModelIndex indexForTextItem(ChatTextItemHelper *textItem) const;

    QPointer<ChatMarkdownHelper> m_markdownHelper;
    QPointer<ChatKeyHelper> m_keyHelper;
    void connectKeyHelper();

    ComponentIt insertComponent(int row, MessageComponentType::Type type, QVariantMap attributes = {}, const QString &intialText = {});
    ComponentIt removeComponent(ComponentIt it);
    void removeComponent(ChatTextItemHelper *textItem);

    void updateCache() const;
    QString messageText() const;

    void clearModel();
};
