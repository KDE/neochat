// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "stickerevent.h"

using namespace Quotient;

StickerEvent::StickerEvent(const QJsonObject &obj)
#ifdef QUOTIENT_07
    : RoomEvent(obj)
#else
    : RoomEvent(typeId(), obj)
#endif
    , m_imageContent(EventContent::ImageContent(obj["content"_ls].toObject()))
{
}

QString StickerEvent::body() const
{
    return content<QString>("body"_ls);
}

const EventContent::ImageContent &StickerEvent::image() const
{
    return m_imageContent;
}

QUrl StickerEvent::url() const
{
#ifdef QUOTIENT_07
    return m_imageContent.url();
#else
    return m_imageContent.url;
#endif
}
