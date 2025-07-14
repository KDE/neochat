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

public:
    enum MediaType {
        Image = 0,
        Video,
    };
    Q_ENUM(MediaType)

    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        SourceRole = MessageFilterModel::LastRole + 1, /**< The mxc source URL for the item. */
        TempSourceRole, /**< Source for the temporary content (either blurhash or mxc URL). */
        TypeRole, /**< The type of the media (image or video). */
        CaptionRole, /**< The caption for the item. */
        SourceWidthRole, /**< The width of the source item. */
        SourceHeightRole, /**< The height of the source item. */
    };
    Q_ENUM(Roles)

    explicit MediaMessageFilterModel(QObject *parent = nullptr, MessageFilterModel *sourceMediaModel = nullptr);

    /**
     * @brief Custom filter to show only image and video messages.
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QSortFilterProxyModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractProxyModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    int getRowForEventId(const QString &eventId) const;
};
