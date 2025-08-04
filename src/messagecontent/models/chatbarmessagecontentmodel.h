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
     * @brief The row of the model component that currently has focus.
     */
    Q_PROPERTY(int focusRow READ focusRow WRITE setFocusRow NOTIFY focusRowChanged)

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

    int focusRow() const;
    Q_INVOKABLE void setFocusRow(int focusRow, bool mouse = false);
    void setFocusIndex(const QModelIndex &index, bool mouse = false);
    Q_INVOKABLE void refocusCurrentComponent() const;

    bool bold() const;
    void setBold(bool bold);
    bool italic() const;
    void setItalic(bool italic);

    Q_INVOKABLE void addNewComponent(MessageComponentType::Type type);

    Q_INVOKABLE void insertComponentAtCursor(MessageComponentType::Type type);

    Q_INVOKABLE void removeComponent(int row, bool removeLast = false);

    Q_INVOKABLE void postMessage();

Q_SIGNALS:
    void focusRowChanged();
    void boldChanged();
    void italicChanged();

private:
    void initializeModel();

    std::optional<QString> getReplyEventId() override;

    ChatDocumentHandler *documentHandlerForComponent(const MessageComponent &component) const;
    ChatDocumentHandler *documentHandlerForIndex(const QModelIndex &index) const;

    ComponentIt insertComponent(int row, MessageComponentType::Type type);

    void focusCurrentComponent(const QModelIndex &previousIndex, bool down);
    ChatDocumentHandler *focusedDocumentHandler() const;
    void emitFocusChangeSignals();

    QString messageText() const;
};
