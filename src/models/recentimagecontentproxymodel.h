// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QSortFilterProxyModel>
#include <QQmlEngine>

class RecentImageContentProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit RecentImageContentProxyModel(QObject *parent = nullptr);

    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;
};
