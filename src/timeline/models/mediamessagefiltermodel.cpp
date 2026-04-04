// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "mediamessagefiltermodel.h"

#include <Quotient/events/roommessageevent.h>
#include <Quotient/room.h>

#include "messagefiltermodel.h"
#include "timelinemessagemodel.h"

using namespace Qt::StringLiterals;

MediaMessageFilterModel::MediaMessageFilterModel(QObject *parent, MessageFilterModel *sourceMediaModel)
    : QSortFilterProxyModel(parent)
{
    Q_ASSERT(sourceMediaModel);
    setSourceModel(sourceMediaModel);

    connect(sourceMediaModel, &MessageFilterModel::selectionChanged, this, &MediaMessageFilterModel::selectionChanged);
}

bool MediaMessageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return sourceModel()->index(sourceRow, 0, sourceParent).data(TimelineMessageModel::IsMediaRole).toBool();
}

const Quotient::RoomEvent *MediaMessageFilterModel::findEvent(const QString &eventId) const
{
    return static_cast<MessageFilterModel *>(sourceModel())->findEvent(eventId);
}

int MediaMessageFilterModel::selectedMessageCount() const
{
    return static_cast<MessageFilterModel *>(sourceModel())->selectedMessageCount();
}

bool MediaMessageFilterModel::isMessageSelected(const QString &eventId) const
{
    return static_cast<MessageFilterModel *>(sourceModel())->isMessageSelected(eventId);
}

void MediaMessageFilterModel::toggleMessageSelection(const QString &eventId)
{
    static_cast<MessageFilterModel *>(sourceModel())->toggleMessageSelection(eventId);
}

int MediaMessageFilterModel::getRowForEventId(const QString &eventId) const
{
    for (auto i = 0; i < rowCount(); i++) {
        if (data(index(i, 0), MessageModel::EventIdRole).toString() == eventId) {
            return i;
        }
    }
    return -1;
}

#include "moc_mediamessagefiltermodel.cpp"
