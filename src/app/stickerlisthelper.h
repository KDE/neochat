// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "neochatroom.h"
#include "stickerlist.h"

namespace Quotient
{
class ImagePackEventContent;
}

class StickerListHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current room from which sticker may be obtained.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief The StickerLists currently available.
     *
     * @sa StickerList
     */
    Q_PROPERTY(QList<StickerList *> stickerLists READ stickerLists NOTIFY stickerListsChanged)

public:
    explicit StickerListHelper(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    [[nodiscard]] QList<StickerList *> stickerLists() const;

Q_SIGNALS:
    void roomChanged();
    void stickerListsChanged();

private:
    QPointer<NeoChatRoom> m_room;
    QList<StickerList *> m_lists;

    void reloadImages();
};
