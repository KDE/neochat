// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "windowmodel.h"

#include <KWindowSystem>

int WindowModel::rowCount(const QModelIndex &parent) const
{
    return KWindowSystem::windows().size();
}
QVariant WindowModel::data(const QModelIndex &index, int role) const
{
    if (role == TitleRole) {
        return KWindowInfo(KWindowSystem::windows()[index.row()], NET::WMName).name();
    } else if (role == IdRole) {
        return KWindowSystem::windows()[index.row()];
    }
    return QStringLiteral("FOO");
}

QHash<int, QByteArray> WindowModel::roleNames() const
{
    return {{TitleRole, "title"}, {IdRole, "id"}};
}

WindowModel::WindowModel()
{
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, [=]() {
        beginResetModel();
        endResetModel();
    });
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, [=]() {
        beginResetModel();
        endResetModel();
    });
}
