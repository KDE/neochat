// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "accountlistmodel.h"

#include "room.h"

AccountListModel::AccountListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&Controller::instance(), &Controller::connectionAdded, this, [=]() {
        beginResetModel();
        endResetModel();
    });
    connect(&Controller::instance(), &Controller::connectionDropped, this, [=]() {
        beginResetModel();
        endResetModel();
    });
}

QVariant AccountListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= Controller::instance().connections().count()) {
        return {};
    }

    auto connection = Controller::instance().connections().at(index.row());

    if (role == UserRole) {
        return QVariant::fromValue(connection->user());
    }
    if (role == ConnectionRole) {
        return QVariant::fromValue(connection);
    }

    return {};
}

int AccountListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return Controller::instance().connections().count();
}

QHash<int, QByteArray> AccountListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[UserRole] = "user";
    roles[ConnectionRole] = "connection";

    return roles;
}
