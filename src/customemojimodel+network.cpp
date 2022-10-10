// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <csapi/account-data.h>
#include <csapi/content-repo.h>

#include "controller.h"
#include "customemojimodel.h"
#include <connection.h>

#ifdef QUOTIENT_07
#define running isJobPending
#else
#define running isJobRunning
#endif

void CustomEmojiModel::fetchEmojis()
{
    if (!Controller::instance().activeConnection()) {
        return;
    }

    const auto &data = Controller::instance().activeConnection()->accountData("im.ponies.user_emotes");
    if (data == nullptr) {
        return;
    }
    QJsonObject emojis = data->contentJson()["images"].toObject();

    // TODO: Remove with stable migration
    const auto legacyEmojis = data->contentJson()["emoticons"].toObject();
    for (const auto &emoji : legacyEmojis.keys()) {
        if (!emojis.contains(emoji)) {
            emojis[emoji] = legacyEmojis[emoji];
        }
    }

    beginResetModel();
    m_emojis.clear();

    for (const auto &emoji : emojis.keys()) {
        const auto &data = emojis[emoji];

        const auto e = emoji.startsWith(":") ? emoji : (QStringLiteral(":") + emoji + QStringLiteral(":"));

        m_emojis << CustomEmoji{e, data.toObject()["url"].toString(), QRegularExpression(QStringLiteral(R"((^|[^\\]))") + e)};
    }

    endResetModel();
}

void CustomEmojiModel::addEmoji(const QString &name, const QUrl &location)
{
    using namespace Quotient;

    auto job = Controller::instance().activeConnection()->uploadFile(location.toLocalFile());

    if (running(job)) {
        connect(job, &BaseJob::success, this, [this, name, job] {
            const auto &data = Controller::instance().activeConnection()->accountData("im.ponies.user_emotes");
            auto json = data != nullptr ? data->contentJson() : QJsonObject();
            auto emojiData = json["images"].toObject();
            emojiData[QStringLiteral("%1").arg(name)] = QJsonObject({
#ifdef QUOTIENT_07
                {QStringLiteral("url"), job->contentUri().toString()}
#else
                {QStringLiteral("url"), job->contentUri()}
#endif
            });
            json["images"] = emojiData;
            Controller::instance().activeConnection()->setAccountData("im.ponies.user_emotes", json);
        });
    }
}

void CustomEmojiModel::removeEmoji(const QString &name)
{
    using namespace Quotient;

    const auto &data = Controller::instance().activeConnection()->accountData("im.ponies.user_emotes");
    Q_ASSERT(data);
    auto json = data->contentJson();
    const QString _name = name.mid(1).chopped(1);
    auto emojiData = json["images"].toObject();

    if (emojiData.contains(name)) {
        emojiData.remove(name);
        json["images"] = emojiData;
    }
    if (emojiData.contains(_name)) {
        emojiData.remove(_name);
        json["images"] = emojiData;
    }
    emojiData = json["emoticons"].toObject();
    if (emojiData.contains(name)) {
        emojiData.remove(name);
        json["emoticons"] = emojiData;
    }
    if (emojiData.contains(_name)) {
        emojiData.remove(_name);
        json["emoticons"] = emojiData;
    }
    Controller::instance().activeConnection()->setAccountData("im.ponies.user_emotes", json);
}
