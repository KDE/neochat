// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QVector>
#include <events/eventcontent.h>
#include <events/stateevent.h>

namespace Quotient
{
/**
 * @class ImagePackEventContent
 *
 * A class to define the content of an image pack event.
 *
 * See Matrix MSC2545 for more details.
 * https://github.com/Sorunome/matrix-doc/blob/soru/emotes/proposals/2545-emotes.md
 *
 * @sa ImagePackEvent
 */
class ImagePackEventContent
{
public:
    /**
     * @brief Defines the properties of an image pack.
     */
    struct Pack {
        Quotient::Omittable<QString> displayName; /**< The display name of the pack. */
        Quotient::Omittable<QUrl> avatarUrl; /**< The source mxc URL for the pack avatar. */
        Quotient::Omittable<QStringList> usage; /**< An array of the usages for this pack. Possible usages are "emoticon" and "sticker". */
        Quotient::Omittable<QString> attribution; /**< The attribution for the pack author(s). */
    };

    /**
     * @brief Defines the properties of an image pack image.
     */
    struct ImagePackImage {
        QString shortcode; /**< The shortcode for the image. */
        QUrl url; /**< The mxc URL for this image. */
        Quotient::Omittable<QString> body; /**< An optional text body for this image. */
        Quotient::Omittable<Quotient::EventContent::ImageInfo> info; /**< The ImageInfo object used for the info block of m.sticker events. */
        /**
         * @brief An array of the usages for this image.
         *
         * The possible values match those of the usage key of a pack object.
         */
        Quotient::Omittable<QStringList> usage;
    };

    /**
     * @brief Return the pack properties.
     *
     * @sa Pack
     */
    Quotient::Omittable<Pack> pack;

    /**
     * @brief Return a vector of images in the pack.
     *
     * @sa ImagePackImage
     */
    QVector<ImagePackEventContent::ImagePackImage> images;

    explicit ImagePackEventContent(const QJsonObject &o);

    /**
     * @brief The definition of how to convert the content to Json.
     *
     * This is a specialization of the standard fillJson function from libQuotient.
     *
     * @sa Quotient::converters
     */
    void fillJson(QJsonObject *o) const;
};

/**
 * @class ImagePackEvent
 *
 * Class to define an image pack state event.
 *
 * The event content is ImagePackEventContent.
 *
 * @sa Quotient::StateEvent, ImagePackEventContent
 */
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
