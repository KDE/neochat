// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

class DevicesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged)

public:
    DevicesProxyModel(QObject *parent = nullptr);
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    void setType(int type);
    [[nodiscard]] int type() const;

Q_SIGNALS:
    void typeChanged();

private:
    int m_type;
};
