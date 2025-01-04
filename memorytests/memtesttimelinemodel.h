// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/events/roomevent.h>
#include <Quotient/syncdata.h>

#include "models/messagemodel.h"

namespace Quotient
{
class Connection;
}

class NeoChatRoom;

class MemTestRoom : public NeoChatRoom
{
public:
    MemTestRoom(Quotient::Connection *connection, const QString &roomName, const QString &syncFileName = {})
        : NeoChatRoom(connection, roomName, Quotient::JoinState::Join)
    {
        syncNewEvents(syncFileName);
    }

    void update(Quotient::SyncRoomData &&data, bool fromCache = false)
    {
        Room::updateData(std::move(data), fromCache);
    }

    void syncNewEvents(const QString &syncFileName)
    {
        if (!syncFileName.isEmpty()) {
            QFile testSyncFile;
            testSyncFile.setFileName(QStringLiteral(DATA_DIR) + u'/' + syncFileName);
            testSyncFile.open(QIODevice::ReadOnly);
            auto testSyncJson = QJsonDocument::fromJson(testSyncFile.readAll()).object();
            Quotient::SyncRoomData roomData(id(), Quotient::JoinState::Join, testSyncJson);
            update(std::move(roomData));
        }
    }
};

/**
 * @class MemTestTimelineModel
 *
 * This is a special version of the MessageModel design to load an unchanging set
 * of events from a json file so that timeline memory optimisations can be measured.
 */
class MemTestTimelineModel : public MessageModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MemTestTimelineModel(QObject *parent = nullptr);

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    QPointer<Quotient::Connection> m_connection;

    std::vector<Quotient::RoomEventPtr> m_events;

    std::optional<std::reference_wrapper<const Quotient::RoomEvent>> getEventForIndex(QModelIndex index) const override;
};
