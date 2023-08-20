// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "imagepackevent.h"
#include <QJsonObject>

using namespace Quotient;

ImagePackEventContent::ImagePackEventContent(const QJsonObject &json)
{
    if (json.contains(QStringLiteral("pack"))) {
        pack = ImagePackEventContent::Pack{
            fromJson<Omittable<QString>>(json["pack"_ls].toObject()["display_name"_ls]),
            fromJson<Omittable<QUrl>>(json["pack"_ls].toObject()["avatar_url"_ls]),
            fromJson<Omittable<QStringList>>(json["pack"_ls].toObject()["usage"_ls]),
            fromJson<Omittable<QString>>(json["pack"_ls].toObject()["attribution"_ls]),
        };
    } else {
        pack = none;
    }

    const auto &keys = json["images"_ls].toObject().keys();
    for (const auto &k : keys) {
        Omittable<EventContent::ImageInfo> info;
        if (json["images"_ls][k].toObject().contains(QStringLiteral("info"))) {
            info = EventContent::ImageInfo(QUrl(json["images"_ls][k]["url"_ls].toString()), json["images"_ls][k]["info"_ls].toObject(), k);
        } else {
            info = none;
        }
        images += ImagePackImage{
            k,
            fromJson<QUrl>(json["images"_ls][k]["url"_ls].toString()),
            fromJson<Omittable<QString>>(json["images"_ls][k]["body"_ls]),
            info,
            fromJson<Omittable<QStringList>>(json["images"_ls][k]["usage"_ls]),
        };
    }
}

void ImagePackEventContent::fillJson(QJsonObject *o) const
{
    if (pack) {
        QJsonObject packJson;
        if (pack->displayName) {
            packJson["display_name"_ls] = *pack->displayName;
        }
        if (pack->usage) {
            QJsonArray usageJson;
            for (const auto &usage : *pack->usage) {
                usageJson += usage;
            }
            packJson["usage"_ls] = usageJson;
        }
        if (pack->avatarUrl) {
            packJson["avatar_url"_ls] = pack->avatarUrl->toString();
        }
        if (pack->attribution) {
            packJson["attribution"_ls] = *pack->attribution;
        }
        (*o)["pack"_ls] = packJson;
    }

    QJsonObject imagesJson;
    for (const auto &image : images) {
        QJsonObject imageJson;
        imageJson["url"_ls] = image.url.toString();
        if (image.body) {
            imageJson["body"_ls] = *image.body;
        }
        if (image.usage) {
            QJsonArray usageJson;
            for (const auto &usage : *image.usage) {
                usageJson += usage;
            }
            imageJson["usage"_ls] = usageJson;
        }
        imagesJson[image.shortcode] = imageJson;
    }
    (*o)["images"_ls] = imagesJson;
}
