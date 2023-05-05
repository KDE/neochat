// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QVector>
#include <events/eventcontent.h>
#include <events/stateevent.h>

namespace Quotient
{
class ImagePackEventContent
{
public:
    struct Pack {
        Quotient::Omittable<QString> displayName;
        Quotient::Omittable<QUrl> avatarUrl;
        Quotient::Omittable<QStringList> usage;
        Quotient::Omittable<QString> attribution;
    };

    struct ImagePackImage {
        QString shortcode;
        QUrl url;
        Quotient::Omittable<QString> body;
        Quotient::Omittable<Quotient::EventContent::ImageInfo> info;
        Quotient::Omittable<QStringList> usage;
    };

    Quotient::Omittable<Pack> pack;
    QVector<ImagePackEventContent::ImagePackImage> images;

    explicit ImagePackEventContent(const QJsonObject &o);

    void fillJson(QJsonObject *o) const;
};

#ifdef QUOTIENT_07
class ImagePackEvent : public KeyedStateEventBase<ImagePackEvent, ImagePackEventContent>
#else
class ImagePackEvent : public StateEvent<ImagePackEventContent>
#endif
{
public:
#ifdef QUOTIENT_07
    QUO_EVENT(ImagePackEvent, "im.ponies.room_emotes")
    using KeyedStateEventBase::KeyedStateEventBase;
#else
    DEFINE_EVENT_TYPEID("im.ponies.room_emotes", ImagePackEvent)
    explicit ImagePackEvent(const QJsonObject &obj)
        : StateEvent(typeId(), obj)
    {
    }
#endif
};

REGISTER_EVENT_TYPE(ImagePackEvent)
}
