// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "messagefiltermodel.h"

#include "messageeventmodel.h"
#include "neochatconfig.h"

using namespace Quotient;

MessageFilterModel::MessageFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowStateEventChanged, this, [this] {
        invalidateFilter();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowLeaveJoinEventChanged, this, [this] {
        invalidateFilter();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowRenameChanged, this, [this] {
        invalidateFilter();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowAvatarUpdateChanged, this, [this] {
        invalidateFilter();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowDeletedMessagesChanged, this, [this] {
        invalidateFilter();
    });
}

bool MessageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.data(MessageEventModel::IsRedactedRole).toBool() && !NeoChatConfig::self()->showDeletedMessages()) {
        return false;
    }

    const int specialMarks = index.data(MessageEventModel::SpecialMarksRole).toInt();
    if (specialMarks == EventStatus::Hidden || specialMarks == EventStatus::Replaced) {
        return false;
    }

    const auto eventType = index.data(MessageEventModel::DelegateTypeRole).toInt();

    if (eventType == MessageEventModel::Other) {
        return false;
    }

    return true;
}
