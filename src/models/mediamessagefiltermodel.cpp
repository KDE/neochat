// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "mediamessagefiltermodel.h"

#include <Quotient/events/roommessageevent.h>
#include <Quotient/room.h>

#include "messagecontentmodel.h"
#include "messagefiltermodel.h"
#include "timelinemessagemodel.h"

using namespace Qt::StringLiterals;

MediaMessageFilterModel::MediaMessageFilterModel(QObject *parent, MessageFilterModel *sourceMediaModel)
    : QSortFilterProxyModel(parent)
{
    Q_ASSERT(sourceMediaModel);
    setSourceModel(sourceMediaModel);
}

bool MediaMessageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.data(TimelineMessageModel::MediaInfoRole).toMap()["mimeType"_L1].toString().contains("image"_L1)
        || index.data(TimelineMessageModel::MediaInfoRole).toMap()["mimeType"_L1].toString().contains("video"_L1)) {
        return true;
    }
    return false;
}

QVariant MediaMessageFilterModel::data(const QModelIndex &index, int role) const
{
    // We need to catch this one and return true if the next media object was
    // on a different day.
    if (role == TimelineMessageModel::ShowSectionRole) {
        const auto day = mapToSource(index).data(TimelineMessageModel::TimeRole).toDateTime().toLocalTime().date();
        const auto previousEventDay = mapToSource(this->index(index.row() + 1, 0)).data(TimelineMessageModel::TimeRole).toDateTime().toLocalTime().date();
        return day != previousEventDay;
    }
    // Catch and force the author to be shown for all rows
    if (role == TimelineMessageModel::ContentModelRole) {
        const auto model = qvariant_cast<MessageContentModel *>(mapToSource(index).data(TimelineMessageModel::ContentModelRole));
        if (model != nullptr) {
            model->setShowAuthor(true);
        }
        return QVariant::fromValue<MessageContentModel *>(model);
    }

    QVariantMap mediaInfo = mapToSource(index).data(TimelineMessageModel::MediaInfoRole).toMap();

    if (role == TempSourceRole) {
        return mediaInfo[u"tempInfo"_s].toMap()[u"source"_s].toUrl();
    }
    if (role == CaptionRole) {
        return mapToSource(index).data(Qt::DisplayRole);
    }
    if (role == SourceWidthRole) {
        return mediaInfo[u"width"_s].toFloat();
    }
    if (role == SourceHeightRole) {
        return mediaInfo[u"height"_s].toFloat();
    }

    bool isVideo = mediaInfo[u"mimeType"_s].toString().contains("video"_L1);

    if (role == TypeRole) {
        return (isVideo) ? MediaType::Video : MediaType::Image;
    }
    if (role == SourceRole) {
        if (isVideo) {
            auto progressInfo = mapToSource(index).data(TimelineMessageModel::ProgressInfoRole).value<Quotient::FileTransferInfo>();
            if (progressInfo.completed()) {
                return mapToSource(index).data(TimelineMessageModel::ProgressInfoRole).value<Quotient::FileTransferInfo>().localPath;
            }
        } else {
            return mediaInfo[u"source"_s].toUrl();
        }
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
