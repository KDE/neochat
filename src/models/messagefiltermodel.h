// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QSortFilterProxyModel>

/**
 * @class MessageFilterModel
 *
 * This model filters out any messages that should be hidden.
 *
 * Deleted messages are only hidden if the user hasn't set them to be shown.
 */
class MessageFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MessageFilterModel(QObject *parent = nullptr);

    /**
     * @brief Custom filter function to remove hidden messages.
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};
