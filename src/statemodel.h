// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>

#include "neochatroom.h"

class StateModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    enum Roles {
        TypeRole,
        StateKeyRole,
        SourceRole,
    };
    Q_ENUM(Roles);

    StateModel(QObject *parent = nullptr);
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

Q_SIGNALS:
    void roomChanged();

private:
    NeoChatRoom *m_room = nullptr;
};
