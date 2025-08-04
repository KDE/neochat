// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <qabstractitemmodel.h>

#include "chatdocumenthandler.h"
#include "enums/messagecomponenttype.h"
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
     * @brief The ChatDocumentHandler of the model component that currently has focus.
     */
    Q_PROPERTY(ChatDocumentHandler *focusedDocumentHandler READ focusedDocumentHandler NOTIFY focusRowChanged)

    /**
     * @brief Set the text style in the currently focused component to bold.
     *
     * If any text is selected the format is appied to it. If not text is selected the format
     * is applied to QTextCursor::WordUnderCursor
     *
     * @sa QTextCursor
     */
    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY boldChanged)

    /**
     * @brief Set the text style in the currently focused component to italic.
     *
     * If any text is selected the format is appied to it. If not text is selected the format
     * is applied to QTextCursor::WordUnderCursor
     *
     * @sa QTextCursor
     */
    Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY italicChanged)

public:
    explicit ChatBarMessageContentModel(QObject *parent = nullptr);

    ChatBarType::Type type() const;
    void setType(ChatBarType::Type type);

    int focusRow() const;
    MessageComponentType::Type focusType() const;
    Q_INVOKABLE void setFocusRow(int focusRow, bool mouse = false);
    void setFocusIndex(const QModelIndex &index, bool mouse = false);
    Q_INVOKABLE void refocusCurrentComponent() const;
    ChatDocumentHandler *focusedDocumentHandler() const;

    bool bold() const;
    void setBold(bool bold);
    bool italic() const;
    void setItalic(bool italic);

    Q_INVOKABLE void insertComponentAtCursor(MessageComponentType::Type type);

    Q_INVOKABLE void addAttachment(const QUrl &path);

    Q_INVOKABLE void removeComponent(int row, bool removeLast = false);

    Q_INVOKABLE void removeAttachment();

    Q_INVOKABLE void postMessage();

Q_SIGNALS:
    void typeChanged();
    void focusRowChanged();
    void boldChanged();
    void italicChanged();

private:
    ChatBarType::Type m_type = ChatBarType::None;

    void initializeModel();

    std::optional<QString> getReplyEventId() override;

    void connectHandler(ChatDocumentHandler *handler);
    ChatDocumentHandler *documentHandlerForComponent(const MessageComponent &component) const;
    ChatDocumentHandler *documentHandlerForIndex(const QModelIndex &index) const;
    QModelIndex indexForDocumentHandler(ChatDocumentHandler *handler) const;
    void updateDocumentHandlerRefs(const ComponentIt &it);

    ComponentIt insertComponent(int row, MessageComponentType::Type type, QVariantMap attributes = {}, const QString &intialText = {});
    void removeComponent(ChatDocumentHandler *handler);

    void focusCurrentComponent(const QModelIndex &previousIndex, bool down);
    void emitFocusChangeSignals();

    void updateCache() const;
    QString messageText() const;

    void clearModel();
};
