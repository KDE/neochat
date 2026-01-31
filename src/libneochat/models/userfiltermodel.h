// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <Quotient/quotient_common.h>

#include <QQmlEngine>
#include <QSortFilterProxyModel>

/**
 * @class UserFilterModel
 *
 * This class creates a custom QSortFilterProxyModel for filtering a users by either
 * display name or matrix ID. The filter can accept a full matrix id i.e. example:kde.org
 * to separate between accounts on different servers with similar names.
 */
class UserFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief This property hold the text of the filter.
     *
     * The text is either a desired display name or matrix id.
     */
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(bool allowEmpty READ allowEmpty WRITE setAllowEmpty NOTIFY allowEmptyChanged)
    /**
     * @brief Only shows users with this membership state.
     */
    Q_PROPERTY(Quotient::Membership membership READ membership WRITE setMembership NOTIFY membershipChanged)
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    /**
     * @brief Custom filter function checking boith the display name and matrix ID.
     *
     * @note The filter cannot be modified and will always use the same filter properties.
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    QString filterText() const;
    void setFilterText(const QString &filterText);

    bool allowEmpty() const;
    void setAllowEmpty(bool allowEmpty);

    Quotient::Membership membership() const;
    void setMembership(Quotient::Membership state);

Q_SIGNALS:
    void filterTextChanged();
    void allowEmptyChanged();
    void membershipChanged();

private:
    QString m_filterText;
    bool m_allowEmpty = false;
    Quotient::Membership m_membership = Quotient::Membership::Invalid;
};
