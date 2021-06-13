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
    const auto emojies = data->contentJson()["emoticons"].toObject();

    beginResetModel();
    d->emojies.clear();

    for (const auto& emoji : emojies.keys()) {
        const auto& data = emojies[emoji];

        d->emojies << CustomEmoji {
            .name = emoji,
            .url = data["url"].toString(),
            .regexp = QRegularExpression(QStringLiteral(R"((^|[^\\]))") + emoji)
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
            auto emojiData = json["emoticons"].toObject();
            emojiData[QStringLiteral(":%1:").arg(name)] = QJsonObject({
                {QStringLiteral("url"), job->contentUri()}
            });
            json["emoticons"] = emojiData;
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
    auto emojiData = json["emoticons"].toObject();
    emojiData.remove(name);
    json["emoticons"] = emojiData;
    d->conn->setAccountData("im.ponies.user_emotes", json);
}
