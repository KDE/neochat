// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "enums/messagecomponenttype.h"
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
     * If any text is selected it is made bold.
     */
    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY boldChanged)

public:
    explicit ChatBarMessageContentModel(QObject *parent = nullptr);

    int focusRow() const;
    void setFocusRow(int focusRow);

    bool bold() const;
    void setBold(bool bold);

    Q_INVOKABLE void addNewComponent(MessageComponentType::Type type);

    Q_INVOKABLE void postMessage();

Q_SIGNALS:
    void focusRowChanged();
    void boldChanged();

private:
    void initializeModel();

    std::optional<QString> getReplyEventId() override;

    QPersistentModelIndex m_currentFocusComponent = {};
    void emitFocusChangeSignals();
};
