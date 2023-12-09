// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <Quotient/syncdata.h>

#include "neochatroom.h"

namespace Quotient
{
class Connection;
}

namespace TestUtils
{
class TestRoom : public NeoChatRoom
{
public:
    TestRoom(Quotient::Connection *connection, const QString &roomName, const QString &syncFileName = {})
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
            testSyncFile.setFileName(QLatin1String(DATA_DIR) + u'/' + syncFileName);
            testSyncFile.open(QIODevice::ReadOnly);
            const auto testSyncJson = QJsonDocument::fromJson(testSyncFile.readAll());
            Quotient::SyncRoomData roomData(id(), Quotient::JoinState::Join, testSyncJson.object());
            update(std::move(roomData));
        }
    }
};
}
