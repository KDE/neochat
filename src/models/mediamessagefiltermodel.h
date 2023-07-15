// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QSortFilterProxyModel>

#include "models/collapsestateproxymodel.h"

/**
 * @class MediaMessageFilterModel
 *
 * This model filters a MessageEventModel for image and video messages.
 *
 * @sa MessageEventModel
 */
class MediaMessageFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        SourceRole = CollapseStateProxyModel::LastRole + 1, /**< The mxc source URL for the item. */
        TempSourceRole, /**< Source for the temporary content (either blurhash or mxc URL). */
        TypeRole, /**< The type of the media (image or video). */
        CaptionRole, /**< The caption for the item. */
        SourceWidthRole, /**< The width of the source item. */
        SourceHeightRole, /**< The height of the source item. */
    };
    Q_ENUM(Roles)

    explicit MediaMessageFilterModel(QObject *parent = nullptr);

    /**
     * @brief Custom filter to show only image and video messages.
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QSortFilterProxyModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractProxyModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int getRowForSourceItem(int sourceRow) const;
};
