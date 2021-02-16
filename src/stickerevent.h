// SDPX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "events/roomevent.h"
#include "events/eventcontent.h"

namespace Quotient {

/// Sticker messages are specialised image messages that are displayed without
/// controls (e.g. no "download" link, or light-box view on click, as would be
/// displayed for for m.image events).
class StickerEvent : public RoomEvent
{
public:
    DEFINE_EVENT_TYPEID("m.sticker", StickerEvent)

    explicit StickerEvent(const QJsonObject &obj);

    /// \brief A textual representation or associated description of the
    /// sticker image.
    ///
    /// This could be the alt text of the original image, or a message to
    /// accompany and further describe the sticker.
    QString body() const;

    /// \brief Metadata about the image referred to in url including a
    /// thumbnail representation.
    const EventContent::ImageContent &image() const;

    /// \brief The URL to the sticker image. This must be a valid mxc:// URI.
    QUrl url() const;
private:
    EventContent::ImageContent m_imageContent;
};
REGISTER_EVENT_TYPE(StickerEvent)
} // namespace Quotient
