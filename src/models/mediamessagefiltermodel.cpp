// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "mediamessagefiltermodel.h"
#include "models/messageeventmodel.h"
#include <room.h>

MediaMessageFilterModel::MediaMessageFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool MediaMessageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.data(MessageEventModel::DelegateTypeRole).toInt() == MessageEventModel::DelegateType::Image
        || index.data(MessageEventModel::DelegateTypeRole).toInt() == MessageEventModel::DelegateType::Video) {
        return true;
    }
    return false;
}

QVariant MediaMessageFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == SourceRole) {
        if (mapToSource(index).data(MessageEventModel::DelegateTypeRole).toInt() == MessageEventModel::DelegateType::Image) {
            return mapToSource(index).data(MessageEventModel::MediaInfoRole).toMap()["source"].toUrl();
        } else if (mapToSource(index).data(MessageEventModel::DelegateTypeRole).toInt() == MessageEventModel::DelegateType::Video) {
            auto progressInfo = mapToSource(index).data(MessageEventModel::ProgressInfoRole).value<Quotient::FileTransferInfo>();

            if (progressInfo.completed()) {
                return mapToSource(index).data(MessageEventModel::ProgressInfoRole).value<Quotient::FileTransferInfo>().localPath;
            } else {
                return QUrl();
            }
        } else {
            return QUrl();
        }
    }
    if (role == TempSourceRole) {
        return mapToSource(index).data(MessageEventModel::MediaInfoRole).toMap()["tempInfo"].toMap()["source"].toUrl();
    }
    if (role == TypeRole) {
        if (mapToSource(index).data(MessageEventModel::DelegateTypeRole).toInt() == MessageEventModel::DelegateType::Image) {
            return 0;
        } else {
            return 1;
        }
    }
    if (role == CaptionRole) {
        return mapToSource(index).data(Qt::DisplayRole);
    }
    if (role == SourceWidthRole) {
        return mapToSource(index).data(MessageEventModel::MediaInfoRole).toMap()["width"].toFloat();
    }
    if (role == SourceHeightRole) {
        return mapToSource(index).data(MessageEventModel::MediaInfoRole).toMap()["height"].toFloat();
    }

    return sourceModel()->data(mapToSource(index), role);
}

QHash<int, QByteArray> MediaMessageFilterModel::roleNames() const
{
    auto roles = sourceModel()->roleNames();
    roles[SourceRole] = "source";
    roles[TempSourceRole] = "tempSource";
    roles[TypeRole] = "type";
    roles[CaptionRole] = "caption";
    roles[SourceWidthRole] = "sourceWidth";
    roles[SourceHeightRole] = "sourceHeight";
    return roles;
}

int MediaMessageFilterModel::getRowForSourceItem(int sourceRow) const
{
    return mapFromSource(sourceModel()->index(sourceRow, 0)).row();
}

#include "moc_mediamessagefiltermodel.cpp"
