// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

/**
 * @class SpaceChildSortFilterModel
 *
 * This class creates a custom QSortFilterProxyModel for filtering and sorting spaces
 * in a SpaceChildrenModel.
 *
 * @sa SpaceChildrenModel
 */
class SpaceChildSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The text to use to filter room names.
     */
    Q_PROPERTY(QString filterText READ filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

public:
    SpaceChildSortFilterModel(QObject *parent = nullptr);

    void setFilterText(const QString &filterText);
    [[nodiscard]] QString filterText() const;

protected:
    /**
     * @brief Returns true if the value of source_left is less than source_right.
     *
     * @sa QSortFilterProxyModel::lessThan
     */
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    /**
     * @brief Custom filter function checking if an event type has been filtered out.
     *
     * The filter rejects a row if the room is known been replaced or if a search
     * string is set it will only return rooms that match.
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    Q_INVOKABLE void move(const QModelIndex &currentIndex, const QModelIndex &targetIndex);

Q_SIGNALS:
    void filterTextChanged();

private:
    QString m_filterText;
};
