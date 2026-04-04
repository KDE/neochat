// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

#include "models/messagefiltermodel.h"

class MessageFilterModel;

/**
 * @class MediaMessageFilterModel
 *
 * This model filters a TimelineMessageModel for image and video messages.
 *
 * @sa TimelineMessageModel
 */
class MediaMessageFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The number of selected messages.
     */
    Q_PROPERTY(int selectedMessageCount READ selectedMessageCount NOTIFY selectionChanged)

public:
    enum MediaType {
        Image = 0,
        Video,
    };
    Q_ENUM(MediaType)

    explicit MediaMessageFilterModel(QObject *parent = nullptr, MessageFilterModel *sourceMediaModel = nullptr);

    /**
     * @brief Custom filter to show only image and video messages.
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    /**
     * @brief Finds the event of the given event ID in the model, returning nullptr if no matches were found.
     */
    Q_INVOKABLE const Quotient::RoomEvent *findEvent(const QString &eventId) const;

    /**
     * @brief The number of selected messages.
     */
    int selectedMessageCount() const;

    /**
     * @brief Whether the given message is selected.
     */
    Q_INVOKABLE bool isMessageSelected(const QString &eventId) const;

    /**
     * @brief Toggle the selection state of the given message.
     */
    Q_INVOKABLE void toggleMessageSelection(const QString &eventId);

    int getRowForEventId(const QString &eventId) const;

Q_SIGNALS:
    /**
     * @brief Emitted when a message is selected or deselected.
     */
    void selectionChanged();
};
