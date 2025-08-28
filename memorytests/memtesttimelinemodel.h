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
            auto ok = testSyncFile.open(QIODevice::ReadOnly);
            if (!ok) {
                qWarning() << "Failed to open" << testSyncFile.fileName() << testSyncFile.errorString();
            }

            auto testSyncJson = QJsonDocument::fromJson(testSyncFile.readAll()).object();
            auto timelineJson = testSyncJson["timeline"_L1].toObject();
            timelineJson["events"_L1] = multiplyEvents(timelineJson["events"_L1].toArray(), 100);
            testSyncJson["timeline"_L1] = timelineJson;
            Quotient::SyncRoomData roomData(id(), Quotient::JoinState::Join, testSyncJson);
            update(std::move(roomData));
        }
    }

    QJsonArray multiplyEvents(QJsonArray events, int factor)
    {
        QJsonArray newArray;
        int eventNum = 0;
        int ts = 0;

        for (int i = 0; i < factor; ++i) {
            for (const auto &event : events) {
                auto eventObject = event.toObject();
                auto contentJson = eventObject["content"_L1].toObject();
                if (contentJson.contains("m.relates_to"_L1)) {
                    auto relatesToJson = contentJson["m.relates_to"_L1].toObject();
                    if (relatesToJson.contains("m.in_reply_to"_L1)) {
                        auto replyJson = relatesToJson["m.in_reply_to"_L1].toObject();
                        const auto currentId = eventObject["event_id"_L1].toInt();
                        const auto currentReplyId = replyJson["event_id"_L1].toInt();
                        replyJson["event_id"_L1] = "$%1:example.org"_L1.arg(QString::number(eventNum - (currentId - currentReplyId)));
                        relatesToJson["m.in_reply_to"_L1] = replyJson;
                    } else if (relatesToJson.contains("event_id"_L1)) {
                        const auto currentId = eventObject["event_id"_L1].toInt();
                        const auto currentRelationId = relatesToJson["event_id"_L1].toInt();
                        relatesToJson["event_id"_L1] = "$%1:example.org"_L1.arg(QString::number(eventNum - (currentId - currentRelationId)));
                    }
                    contentJson["m.relates_to"_L1] = relatesToJson;
                    eventObject["content"_L1] = contentJson;
                }
                eventObject["event_id"_L1] = "$%1:example.org"_L1.arg(QString::number(eventNum));
                eventObject["origin_server_ts"_L1] = ts;
                auto unsignedJson = eventObject["unsigned"_L1].toObject();
                unsignedJson["age"_L1] = ts;
                eventObject["unsigned"_L1] = unsignedJson;
                newArray.append(eventObject);
                ++eventNum;
                ++ts;
            }
        }
        return newArray;
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
