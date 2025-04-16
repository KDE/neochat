// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "spacechildsortfiltermodel.h"

#include "spacechildrenmodel.h"

SpaceChildSortFilterModel::SpaceChildSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
    sort(0);
}

void SpaceChildSortFilterModel::setFilterText(const QString &filterText)
{
    m_filterText = filterText;
    Q_EMIT filterTextChanged();
    invalidateFilter();
}

QString SpaceChildSortFilterModel::filterText() const
{
    return m_filterText;
}

bool SpaceChildSortFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (source_left.data(SpaceChildrenModel::IsSpaceRole).toBool() && source_right.data(SpaceChildrenModel::IsSpaceRole).toBool()) {
        if (!source_left.data(SpaceChildrenModel::OrderRole).toString().isEmpty() && !source_right.data(SpaceChildrenModel::OrderRole).toString().isEmpty()) {
            return QString::compare(source_left.data(SpaceChildrenModel::OrderRole).toString(), source_right.data(SpaceChildrenModel::OrderRole).toString())
                < 0;
        }
        return source_left.data(SpaceChildrenModel::ChildTimestampRole).toDateTime() > source_right.data(SpaceChildrenModel::ChildTimestampRole).toDateTime();
    }
    if (source_left.data(SpaceChildrenModel::IsSpaceRole).toBool()) {
        return true;
    } else if (source_right.data(SpaceChildrenModel::IsSpaceRole).toBool()) {
        return false;
    }
    if (!source_left.data(SpaceChildrenModel::OrderRole).toString().isEmpty() && !source_right.data(SpaceChildrenModel::OrderRole).toString().isEmpty()) {
        return QString::compare(source_left.data(SpaceChildrenModel::OrderRole).toString(), source_right.data(SpaceChildrenModel::OrderRole).toString()) < 0;
    }
    if (!source_left.data(SpaceChildrenModel::OrderRole).toString().isEmpty()) {
        return true;
    } else if (!source_right.data(SpaceChildrenModel::OrderRole).toString().isEmpty()) {
        return false;
    }
    return source_left.data(SpaceChildrenModel::ChildTimestampRole).toDateTime() > source_right.data(SpaceChildrenModel::ChildTimestampRole).toDateTime();
}

bool SpaceChildSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (auto sourceModel = static_cast<SpaceChildrenModel *>(this->sourceModel())) {
        bool isReplaced = sourceModel->isRoomReplaced(index.data(SpaceChildrenModel::RoomIDRole).toString());
        bool acceptRoom = index.data(SpaceChildrenModel::DisplayNameRole).toString().contains(m_filterText, Qt::CaseInsensitive);
        return !isReplaced && acceptRoom;
    }
    return true;
}

void SpaceChildSortFilterModel::move(const QModelIndex &currentIndex, const QModelIndex &targetIndex)
{
    const auto rootSpace = dynamic_cast<SpaceChildrenModel *>(sourceModel())->space();
    if (rootSpace == nullptr) {
        return;
    }

    const auto connection = rootSpace->connection();

    const auto currentParent = currentIndex.parent();
    auto targetParent = targetIndex.parent();
    NeoChatRoom *currentParentSpace = nullptr;
    if (!currentParent.isValid()) {
        currentParentSpace = rootSpace;
    } else {
        currentParentSpace = static_cast<NeoChatRoom *>(connection->room(currentParent.data(SpaceChildrenModel::RoomIDRole).toString()));
    }
    NeoChatRoom *targetParentSpace = nullptr;
    if (!targetParent.isValid()) {
        targetParentSpace = rootSpace;
    } else {
        targetParentSpace = static_cast<NeoChatRoom *>(connection->room(targetParent.data(SpaceChildrenModel::RoomIDRole).toString()));
    }
    // If both parents are not resolvable to a room object we don't have the permissions
    // required for this action.
    if (currentParentSpace == nullptr || targetParentSpace == nullptr) {
        return;
    }

    const auto currentRow = currentIndex.row();
    auto targetRow = targetIndex.row();

    const auto moveRoomId = currentIndex.data(SpaceChildrenModel::RoomIDRole).toString();
    auto targetRoom = static_cast<NeoChatRoom *>(connection->room(targetIndex.data(SpaceChildrenModel::RoomIDRole).toString()));
    // If the target room is a space, assume we want to drop the room into it.
    if (targetRoom != nullptr && targetRoom->isSpace()) {
        targetParent = targetIndex;
        targetParentSpace = targetRoom;
        targetRow = rowCount(targetParent);
    }

    const auto newRowCount = rowCount(targetParent) + (currentParentSpace != targetParentSpace ? 1 : 0);
    for (int i = 0; i < newRowCount; i++) {
        if (currentParentSpace == targetParentSpace && i == currentRow) {
            continue;
        }

        targetParentSpace->setChildOrder(index(i, 0, targetParent).data(SpaceChildrenModel::RoomIDRole).toString(),
                                         QString::number(i > targetRow ? i + 1 : i, 36));

        if (i == targetRow) {
            if (currentParentSpace != targetParentSpace) {
                currentParentSpace->removeChild(moveRoomId, true);
                targetParentSpace->addChild(moveRoomId, true, false, false, QString::number(i + 1, 36));
            } else {
                targetParentSpace->setChildOrder(currentIndex.data(SpaceChildrenModel::RoomIDRole).toString(), QString::number(i + 1, 36));
            }
        }
    }
}

#include "moc_spacechildsortfiltermodel.cpp"
