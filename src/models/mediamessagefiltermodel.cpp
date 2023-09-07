// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "mediamessagefiltermodel.h"

#include <Quotient/room.h>

#include "enums/delegatetype.h"
#include "messageeventmodel.h"
#include "messagefiltermodel.h"

MediaMessageFilterModel::MediaMessageFilterModel(QObject *parent, MessageFilterModel *sourceMediaModel)
    : QSortFilterProxyModel(parent)
{
    Q_ASSERT(sourceMediaModel);
    setSourceModel(sourceMediaModel);
}

bool MediaMessageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.data(MessageEventModel::DelegateTypeRole).toInt() == DelegateType::Image
        || index.data(MessageEventModel::DelegateTypeRole).toInt() == DelegateType::Video) {
        return true;
    }
    return false;
}

QVariant MediaMessageFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == SourceRole) {
        if (mapToSource(index).data(MessageEventModel::DelegateTypeRole).toInt() == DelegateType::Image) {
            return mapToSource(index).data(MessageEventModel::MediaInfoRole).toMap()[QStringLiteral("source")].toUrl();
        } else if (mapToSource(index).data(MessageEventModel::DelegateTypeRole).toInt() == DelegateType::Video) {
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
        return mapToSource(index).data(MessageEventModel::MediaInfoRole).toMap()[QStringLiteral("tempInfo")].toMap()[QStringLiteral("source")].toUrl();
    }
    if (role == TypeRole) {
        if (mapToSource(index).data(MessageEventModel::DelegateTypeRole).toInt() == DelegateType::Image) {
            return MediaType::Image;
        } else {
            return MediaType::Video;
        }
    }
    if (role == CaptionRole) {
        return mapToSource(index).data(Qt::DisplayRole);
    }
    if (role == SourceWidthRole) {
        return mapToSource(index).data(MessageEventModel::MediaInfoRole).toMap()[QStringLiteral("width")].toFloat();
    }
    if (role == SourceHeightRole) {
        return mapToSource(index).data(MessageEventModel::MediaInfoRole).toMap()[QStringLiteral("height")].toFloat();
    }
    // We need to catch this one and return true if the next media object was
    // on a different day.
    if (role == MessageEventModel::ShowSectionRole) {
        const auto day = mapToSource(index).data(MessageEventModel::TimeRole).toDateTime().toLocalTime().date();
        const auto previousEventDay = mapToSource(this->index(index.row() + 1, 0)).data(MessageEventModel::TimeRole).toDateTime().toLocalTime().date();
        return day != previousEventDay;
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
