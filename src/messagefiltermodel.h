/*
 * SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QSortFilterProxyModel>

class MessageFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

};
