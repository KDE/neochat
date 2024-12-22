// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "imagepackevent.h"
#include <QJsonObject>

using namespace Quotient;

ImagePackEventContent::ImagePackEventContent(const QJsonObject &json)
{
    if (json.contains("pack"_L1)) {
        pack = ImagePackEventContent::Pack{
            fromJson<std::optional<QString>>(json["pack"_L1].toObject()["display_name"_L1]),
            fromJson<std::optional<QUrl>>(json["pack"_L1].toObject()["avatar_url"_L1]),
            fromJson<std::optional<QStringList>>(json["pack"_L1].toObject()["usage"_L1]),
            fromJson<std::optional<QString>>(json["pack"_L1].toObject()["attribution"_L1]),
        };
    } else {
        pack = std::nullopt;
    }

    const auto &keys = json["images"_L1].toObject().keys();
    for (const auto &k : keys) {
        std::optional<EventContent::ImageInfo> info;
        if (json["images"_L1][k].toObject().contains("info"_L1)) {
            info = EventContent::ImageInfo(QUrl(json["images"_L1][k]["url"_L1].toString()), json["images"_L1][k]["info"_L1].toObject(), k);
        } else {
            info = std::nullopt;
        }
        images += ImagePackImage{
            k,
            fromJson<QUrl>(json["images"_L1][k]["url"_L1].toString()),
            fromJson<std::optional<QString>>(json["images"_L1][k]["body"_L1]),
            info,
            fromJson<std::optional<QStringList>>(json["images"_L1][k]["usage"_L1]),
        };
    }
}

void ImagePackEventContent::fillJson(QJsonObject *o) const
{
    if (pack) {
        QJsonObject packJson;
        if (pack->displayName) {
            packJson["display_name"_L1] = *pack->displayName;
        }
        if (pack->usage) {
            QJsonArray usageJson;
            for (const auto &usage : *pack->usage) {
                usageJson += usage;
            }
            packJson["usage"_L1] = usageJson;
        }
        if (pack->avatarUrl) {
            packJson["avatar_url"_L1] = pack->avatarUrl->toString();
        }
        if (pack->attribution) {
            packJson["attribution"_L1] = *pack->attribution;
        }
        (*o)["pack"_L1] = packJson;
    }

    QJsonObject imagesJson;
    for (const auto &image : images) {
        QJsonObject imageJson;
        imageJson["url"_L1] = image.url.toString();
        if (image.body) {
            imageJson["body"_L1] = *image.body;
        }
        if (image.usage) {
            QJsonArray usageJson;
            for (const auto &usage : *image.usage) {
                usageJson += usage;
            }
            imageJson["usage"_L1] = usageJson;
        }
        if (image.info.has_value()) {
            imageJson["info"_L1] = Quotient::EventContent::toInfoJson(*image.info);
        }
        imagesJson[image.shortcode] = imageJson;
    }
    (*o)["images"_L1] = imagesJson;
}
