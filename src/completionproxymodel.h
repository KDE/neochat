// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QSortFilterProxyModel>

class CompletionProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int secondaryFilterRole READ secondaryFilterRole WRITE setSecondaryFilterRole NOTIFY secondaryFilterRoleChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    int secondaryFilterRole() const;
    void setSecondaryFilterRole(int role);

    QString filterText() const;
    void setFilterText(const QString &filterText);

    void setFullText(const QString &fullText);

Q_SIGNALS:
    void secondaryFilterRoleChanged();
    void filterTextChanged();

private:
    int m_secondaryFilterRole = -1;
    QString m_filterText;
    QString m_fullText;
};
