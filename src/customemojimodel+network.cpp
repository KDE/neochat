// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <csapi/account-data.h>
#include <csapi/profile.h>
#include <csapi/content-repo.h>

#include "customemojimodel_p.h"

#ifdef QUOTIENT_07
#define running isJobPending
#else
#define running isJobRunning
#endif

void CustomEmojiModel::fetchEmojies()
{
    if (d->conn == nullptr) {
        return;
    }

    const auto& data = d->conn->accountData("im.ponies.user_emotes");
    if (data == nullptr) {
        return;
    }
    QJsonObject emojies = data->contentJson()["images"].toObject();

    //TODO: Remove with stable migration
    const auto legacyEmojies = data->contentJson()["emoticons"].toObject();
    for(const auto &emoji : legacyEmojies.keys()) {
        if(!emojies.contains(emoji)) {
            emojies[emoji] = legacyEmojies[emoji];
        }
    }

    beginResetModel();
    d->emojies.clear();

    for (const auto& emoji : emojies.keys()) {
        const auto& data = emojies[emoji];

        const auto e = emoji.startsWith(":") ? emoji : (QStringLiteral(":") + emoji + QStringLiteral(":"));

        d->emojies << CustomEmoji {
            e,
            data.toObject()["url"].toString(),
            QRegularExpression(QStringLiteral(R"((^|[^\\]))") + e)
        };
    }

    endResetModel();
}

void CustomEmojiModel::addEmoji(const QString& name, const QUrl& location)
{
    using namespace Quotient;

    auto job = d->conn->uploadFile(location.toLocalFile());

    if (running(job)) {
        connect(job, &BaseJob::success, this, [this, name, job] {
            const auto& data = d->conn->accountData("im.ponies.user_emotes");
            auto json = data != nullptr ? data->contentJson() : QJsonObject();
            auto emojiData = json["images"].toObject();
            emojiData[QStringLiteral("%1").arg(name)] = QJsonObject({
                {QStringLiteral("url"), job->contentUri()}
            });
            json["images"] = emojiData;
            d->conn->setAccountData("im.ponies.user_emotes", json);
        });
    }
}

void CustomEmojiModel::removeEmoji(const QString& name)
{
    using namespace Quotient;

    const auto& data = d->conn->accountData("im.ponies.user_emotes");
    Q_ASSERT(data != nullptr); // something's screwed if we get here with a nullptr
    auto json = data->contentJson();
    const QString _name = name.mid(1).chopped(1);
    auto emojiData = json["images"].toObject();

    if(emojiData.contains(name)) {
        emojiData.remove(name);
        json["images"] = emojiData;
    }
    if(emojiData.contains(_name)) {
        emojiData.remove(_name);
        json["images"] = emojiData;
    }
    emojiData = json["emoticons"].toObject();
    if(emojiData.contains(name)) {
        emojiData.remove(name);
        json["emoticons"] = emojiData;
    }
    if(emojiData.contains(_name)) {
        emojiData.remove(_name);
        json["emoticons"] = emojiData;
    }
    d->conn->setAccountData("im.ponies.user_emotes", json);
}
