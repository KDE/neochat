// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QConcatenateTablesProxyModel>

#include "events/imagepackevent.h"
#include <QAbstractListModel>
#include <QList>
#include <QPointer>
#include <QQmlEngine>

#include "neochatroom.h"

class RoomImagePacksModel;

class ImagePacksModel : public QConcatenateTablesProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(NeoChatRoom *currentRoom READ currentRoom WRITE setCurrentRoom NOTIFY currentRoomChanged)

public:
    ImagePacksModel(QObject *parent = nullptr);
    QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] NeoChatRoom *currentRoom() const;
    void setCurrentRoom(NeoChatRoom *currentRoom);

Q_SIGNALS:
    void currentRoomChanged();

private:
    RoomImagePacksModel *m_roomImagePacksModel = nullptr;
};
