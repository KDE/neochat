// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "call/callparticipant.h"
#include "neochatuser.h"

class CallParticipantsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        WidthRole,
        HeightRole,
        PadRole,
        ObjectRole,
    };
    Q_ENUM(Roles);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    ;

    void addParticipant(CallParticipant *callParticipant);
    CallParticipant *callParticipantForUser(NeoChatUser *user);

    void setHasCamera(NeoChatUser *user, bool hasCamera);
    void clear();

private:
    QVector<CallParticipant *> m_callParticipants;
};