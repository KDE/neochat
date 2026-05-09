// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "mediamessagefiltermodel.h"

#include <Quotient/events/roommessageevent.h>
#include <Quotient/room.h>

#include "block.h"
#include "enums/blocktype.h"
#include "eventhandler.h"
#include "messagefiltermodel.h"
#include "messagemodel.h"
#include "neochatdatetime.h"
#include "timelinemodel.h"

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

QVariant MediaMessageFilterModel::data(const QModelIndex &index, int role) const
{
    // We need to catch this one and return true if the next media object was
    // on a different day.
    if (role == TimelineMessageModel::ShowSectionRole) {
        const auto day = mapToSource(index).data(TimelineMessageModel::DateTimeRole).value<NeoChatDateTime>().dateTime().toLocalTime().date();
        const auto previousEventDay =
        mapToSource(this->index(index.row() + 1, 0)).data(TimelineMessageModel::DateTimeRole).value<NeoChatDateTime>().dateTime().toLocalTime().date();
        return day != previousEventDay;
    }

    const auto filterModel = dynamic_cast<MessageFilterModel *>(sourceModel());
    if (!filterModel) {
        return {};
    }
    const auto messageModel = dynamic_cast<TimelineModel *>(filterModel->sourceModel());
    if (!messageModel || !messageModel->room()) {
        return {};
    }
    const auto event = messageModel->room()->getEvent(mapToSource(index).data(MessageModel::EventIdRole).toString()).first;
    const auto block = EventHandler::blockForMediaEvent(messageModel->room(), event);
    if (!block) {
        return {};
    }

    if (block->type() == Blocks::Image) {
        const auto imageBlock = dynamic_cast<Blocks::ImageBlock *>(block);
        if (!imageBlock) {
            return {};
        }

        if (role == TempSourceRole) {
            return imageBlock->thumbnailSource();
        }
        if (role == CaptionRole) {
            return mapToSource(index).data(Qt::DisplayRole);
        }
        if (role == SourceWidthRole) {
            return imageBlock->info().pixelSize.width();
        }
        if (role == SourceHeightRole) {
            return imageBlock->info().pixelSize.height();
        }
        if (role == TypeRole) {
            return MediaType::Image;
        }
        if (role == SourceRole) {
            return imageBlock->source();
        }
    } else if (block->type() == Blocks::Video) {
        const auto videoBlock = dynamic_cast<Blocks::VideoBlock *>(block);
        if (!videoBlock) {
            return {};
        }

        if (role == TempSourceRole) {
            return videoBlock->thumbnailSource();
        }
        if (role == CaptionRole) {
            return mapToSource(index).data(Qt::DisplayRole);
        }
        if (role == SourceWidthRole) {
            return videoBlock->info().pixelSize.width();
        }
        if (role == SourceHeightRole) {
            return videoBlock->info().pixelSize.height();
        }
        if (role == TypeRole) {
            return MediaType::Video;
        }
        if (role == SourceRole) {
            auto progressInfo = mapToSource(index).data(TimelineMessageModel::ProgressInfoRole).value<Quotient::FileTransferInfo>();
            if (progressInfo.completed()) {
                return mapToSource(index).data(TimelineMessageModel::ProgressInfoRole).value<Quotient::FileTransferInfo>().localPath;
            }
        }
    }

    block->deleteLater();

    return this->sourceModel()->data(mapToSource(index), role);
}

QHash<int, QByteArray> MediaMessageFilterModel::roleNames() const
{
    auto roles = sourceModel() ? sourceModel()->roleNames() : QHash<int, QByteArray>();
    roles[SourceRole] = "source";
    roles[TempSourceRole] = "tempSource";
    roles[TypeRole] = "type";
    roles[CaptionRole] = "caption";
    roles[SourceWidthRole] = "sourceWidth";
    roles[SourceHeightRole] = "sourceHeight";
    return roles;
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

void MediaMessageFilterModel::hideMedia(const QString &eventId)
{
    static_cast<MessageFilterModel *>(sourceModel())->showMedia(eventId);
}

void MediaMessageFilterModel::showMedia(const QString &eventId)
{
    static_cast<MessageFilterModel *>(sourceModel())->hideMedia(eventId);
}

bool MediaMessageFilterModel::isMediaHidden(const QString &eventId)
{
    return static_cast<MessageFilterModel *>(sourceModel())->isMediaHidden(eventId);
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
