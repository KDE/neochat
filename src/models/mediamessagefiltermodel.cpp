// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "mediamessagefiltermodel.h"

#include <Quotient/events/roommessageevent.h>
#include <Quotient/room.h>

#include "messagecontentmodel.h"
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

    if (index.data(MessageEventModel::MediaInfoRole).toMap()[QLatin1String("mimeType")].toString().contains(QLatin1String("image"))
        || index.data(MessageEventModel::MediaInfoRole).toMap()[QLatin1String("mimeType")].toString().contains(QLatin1String("video"))) {
        return true;
    }
    return false;
}

QVariant MediaMessageFilterModel::data(const QModelIndex &index, int role) const
{
    // We need to catch this one and return true if the next media object was
    // on a different day.
    if (role == MessageEventModel::ShowSectionRole) {
        const auto day = mapToSource(index).data(MessageEventModel::TimeRole).toDateTime().toLocalTime().date();
        const auto previousEventDay = mapToSource(this->index(index.row() + 1, 0)).data(MessageEventModel::TimeRole).toDateTime().toLocalTime().date();
        return day != previousEventDay;
    }
    // Catch and force the author to be shown for all rows
    if (role == MessageEventModel::ContentModelRole) {
        const auto model = qvariant_cast<MessageContentModel *>(mapToSource(index).data(MessageEventModel::ContentModelRole));
        if (model != nullptr) {
            model->setShowAuthor(true);
        }
        return QVariant::fromValue<MessageContentModel *>(model);
    }

    QVariantMap mediaInfo = mapToSource(index).data(MessageEventModel::MediaInfoRole).toMap();

    if (role == TempSourceRole) {
        return mediaInfo[QStringLiteral("tempInfo")].toMap()[QStringLiteral("source")].toUrl();
    }
    if (role == CaptionRole) {
        return mapToSource(index).data(Qt::DisplayRole);
    }
    if (role == SourceWidthRole) {
        return mediaInfo[QStringLiteral("width")].toFloat();
    }
    if (role == SourceHeightRole) {
        return mediaInfo[QStringLiteral("height")].toFloat();
    }

    bool isVideo = mediaInfo[QStringLiteral("mimeType")].toString().contains(QStringLiteral("video"));

    if (role == TypeRole) {
        return (isVideo) ? MediaType::Video : MediaType::Image;
    }
    if (role == SourceRole) {
        if (isVideo) {
            auto progressInfo = mapToSource(index).data(MessageEventModel::ProgressInfoRole).value<Quotient::FileTransferInfo>();
            if (progressInfo.completed()) {
                return mapToSource(index).data(MessageEventModel::ProgressInfoRole).value<Quotient::FileTransferInfo>().localPath;
            }
        } else {
            return mediaInfo[QStringLiteral("source")].toUrl();
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
