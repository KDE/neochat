// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "imagepackevent.h"
#include <QJsonObject>

using namespace Quotient;

ImagePackEventContent::ImagePackEventContent(const QJsonObject &json)
{
    if (json.contains(QStringLiteral("pack"))) {
        pack = ImagePackEventContent::Pack{
            fromJson<Omittable<QString>>(json["pack"].toObject()["display_name"]),
#ifdef QUOTIENT_07
            fromJson<Omittable<QUrl>>(json["pack"].toObject()["avatar_url"]),
#else
            QUrl(),
#endif
            fromJson<Omittable<QStringList>>(json["pack"].toObject()["usage"]),
            fromJson<Omittable<QString>>(json["pack"].toObject()["attribution"]),
        };
    } else {
        pack = none;
    }

    const auto &keys = json["images"].toObject().keys();
    for (const auto &k : keys) {
        Omittable<EventContent::ImageInfo> info;
        if (json["images"][k].toObject().contains(QStringLiteral("info"))) {
#ifdef QUOTIENT_07
            info = EventContent::ImageInfo(QUrl(json["images"][k]["url"].toString()), json["images"][k]["info"].toObject(), k);
#else
            info = EventContent::ImageInfo(QUrl(json["images"][k]["url"].toString()), json["images"][k].toObject(), k);
#endif
        } else {
            info = none;
        }
        images += ImagePackImage{
            k,
#ifdef QUOTIENT_07
            fromJson<QUrl>(json["images"][k]["url"].toString()),
#else
            QUrl(),
#endif
            fromJson<Omittable<QString>>(json["images"][k]["body"]),
            info,
            fromJson<Omittable<QStringList>>(json["images"][k]["usage"]),
        };
    }
}

void ImagePackEventContent::fillJson(QJsonObject *o) const
{
    if (pack) {
        QJsonObject packJson;
        if (pack->displayName) {
            packJson["display_name"] = *pack->displayName;
        }
        if (pack->usage) {
            QJsonArray usageJson;
            for (const auto &usage : *pack->usage) {
                usageJson += usage;
            }
            packJson["usage"] = usageJson;
        }
        if (pack->avatarUrl) {
            packJson["avatar_url"] = pack->avatarUrl->toString();
        }
        if (pack->attribution) {
            packJson["attribution"] = *pack->attribution;
        }
        (*o)["pack"_ls] = packJson;
    }

    QJsonObject imagesJson;
    for (const auto &image : images) {
        QJsonObject imageJson;
        imageJson["url"] = image.url.toString();
        if (image.body) {
            imageJson["body"] = *image.body;
        }
        if (image.usage) {
            QJsonArray usageJson;
            for (const auto &usage : *image.usage) {
                usageJson += usage;
            }
            imageJson["usage"] = usageJson;
        }
        imagesJson[image.shortcode] = imageJson;
    }
    (*o)["images"_ls] = imagesJson;
}
