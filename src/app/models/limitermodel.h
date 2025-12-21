// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

/**
 * @class LimiterModel
 *
 * @brief Takes a source QAbstractItemModel model and only displays a desired maximum amount.
 *
 * Also gives you the remaining (filtered out) items, useful for sticking in a label or somesuch.
 */
class LimiterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int maximumCount READ maximumCount WRITE setMaximumCount NOTIFY maximumCountChanged)
    Q_PROPERTY(int extraCount READ extraCount NOTIFY extraCountChanged)

public:
    explicit LimiterModel(QObject *parent = nullptr);

    [[nodiscard]] int maximumCount() const;
    void setMaximumCount(int maximumCount);

    [[nodiscard]] int extraCount() const;

Q_SIGNALS:
    void maximumCountChanged();
    void extraCountChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    int m_maximumCount = 0;
};
