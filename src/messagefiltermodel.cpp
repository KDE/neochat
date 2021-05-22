// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "messagefiltermodel.h"

#include "messageeventmodel.h"
#include "neochatconfig.h"

bool MessageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const int specialMarks = index.data(MessageEventModel::SpecialMarksRole).toInt();

    if (specialMarks == EventStatus::Hidden || specialMarks == EventStatus::Replaced) {
        return false;
    }

    const QString eventType = index.data(MessageEventModel::EventTypeRole).toString();

    if (eventType == QLatin1String("other")) {
        return false;
    }

    if (!NeoChatConfig::self()->showLeaveJoinEvent() && eventType == QLatin1String("state")) {
        return false;
    }

    return true;
}
