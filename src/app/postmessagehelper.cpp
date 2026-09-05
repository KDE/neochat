// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "postmessagehelper.h"

#include <QBuffer>
#include <QMediaRecorder>
#include <QStandardPaths>

#include <KFormat>

#include "actionsmodel.h"
#include "blockreply.h"
#include "events/pollevent.h"
#include "texthandler.h"

PostMessageHelper::PostMessageHelper(QObject *parent)
    : QObject(parent)
{
}

NeoChatRoom *PostMessageHelper::room() const
{
    return m_room;
}

void PostMessageHelper::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }
    m_room = room;
    Q_EMIT roomChanged();
}

Blocks::Cache *PostMessageHelper::cache() const
{
    return m_cache;
}

void PostMessageHelper::setCache(Blocks::Cache *cache)
{
    if (cache == m_cache) {
        return;
    }
    m_cache = cache;
    Q_EMIT cacheChanged();
}

QString PostMessageHelper::editId() const
{
    return m_editId;
}

void PostMessageHelper::setEditId(const QString &editId)
{
    if (editId == m_editId) {
        return;
    }
    m_editId = editId;
    Q_EMIT editIdChanged();
}

QString PostMessageHelper::threadRootId() const
{
    return m_threadRootId;
}

void PostMessageHelper::setThreadRootId(const QString &threadRootId)
{
    if (threadRootId == m_threadRootId) {
        return;
    }
    m_threadRootId = threadRootId;
    Q_EMIT threadRootIdChanged();
}

void PostMessageHelper::postMessage()
{
    if (!m_room || !m_cache) {
        return;
    }

    bool isReply = m_cache->at(0)->type == Blocks::Reply;
    QString replyId;
    if (const auto replyCacheItem = dynamic_cast<const Blocks::ReplyCacheItem *>(m_cache->at(0))) {
        replyId = replyCacheItem->blockModel->eventId();
    }
    std::optional<Quotient::EventRelation> relatesTo = std::nullopt;

    if (!m_threadRootId.isEmpty()) {
        relatesTo = Quotient::EventRelation::replyInThread(m_threadRootId, !isReply, isReply ? replyId : m_threadRootId);
    } else if (!m_editId.isEmpty()) {
        relatesTo = Quotient::EventRelation::replace(m_editId);
    } else if (isReply) {
        relatesTo = Quotient::EventRelation::replyTo(replyId);
    }

    if (Blocks::isFileType(m_cache->at(0)->type)) {
        QUrl source;
        QString filename;
        if (const auto imageItem = dynamic_cast<const Blocks::ImageCacheItem *>(m_cache->at(0)); imageItem != nullptr) {
            if (imageItem->optimize) {
                QImage image(imageItem->source.toLocalFile());

                // Maximum resolution we want to send in standard quality.
                constexpr QSize maximumResolution(3000, 3000);
                // The file format we want standard quality images to be in.
                const auto fileExtension = QStringLiteral("jpg");

                if (image.size().width() > maximumResolution.width() || image.size().height() > maximumResolution.height()) {
                    image = image.scaled(maximumResolution, Qt::AspectRatioMode::KeepAspectRatio);
                }

                QString imageDir(u"%1/optimized"_s.arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));
                if (!QDir().exists(imageDir)) {
                    QDir().mkdir(imageDir);
                }
                QString newFilename = u"%1.%3"_s.arg(QDateTime::currentDateTime().toString(u"yyyy-MM-dd-hh-mm-ss"_s), fileExtension);
                source = QUrl(u"file://%1/%2"_s.arg(imageDir, newFilename));
                // Only override if the old filename was the actual filename, and not custom text entered by the user.
                if (imageItem->source.fileName() == m_cache->toString()) {
                    filename = newFilename;
                } else {
                    filename = m_cache->toString();
                }
                if (!image.save(source.toLocalFile())) {
                    qWarning() << "Failed to save optimized image to" << source << "falling back to the actual source file";
                    source = imageItem->source;
                    filename = m_cache->toString();
                }
            } else {
                source = imageItem->source;
                filename = m_cache->toString();
            }
        } else {
            const auto fileCacheItem = dynamic_cast<const Blocks::UrlCacheItem *>(m_cache->at(0));
            source = fileCacheItem->source;
            filename = m_cache->toString();
        }

        m_room->uploadFile(source, filename, relatesTo);
        m_cache->clear();
        return;
    }

    if (m_cache->hasType(Blocks::Location)) {
        int locationIndex = m_cache->at(0)->type == Blocks::Location ? 0 : 1;
        if (const auto locationItem = m_cache->at<Blocks::LocationCacheItem>(locationIndex)) {
            m_room->sendLocation(locationItem->latitude, locationItem->longitude, m_cache->toString());
            m_cache->clear();
            return;
        }
    }

    const auto [sendString, msgType] = ActionsModel::handleAction(m_room, m_cache->toString(), replyId, m_editId);
    if (!msgType.has_value()) {
        m_cache->clear();
        return;
    }

    TextHandler textHandler;
    textHandler.setData(*sendString);
    const auto sendText = textHandler.handleSendText();

    if (sendText.length() == 0) {
        return;
    }

    auto content = std::make_unique<Quotient::EventContent::TextContent>(sendText, u"text/html"_s);
    auto body = TextHandler::stripMatrixLinks(m_cache->toString());
    body = TextHandler::unescapeBackslashes(body);
    // We want to strip Matrix links here because it matches Element behavior, but more importantly is less annoying in bridged chats.
    m_room->post<Quotient::RoomMessageEvent>(body, *msgType, std::move(content), relatesTo);
    m_cache->clear();
}

void PostMessageHelper::postPoll(PollKind::Kind kind, const QString &question, const QList<QString> &answers)
{
    if (!m_room) {
        return;
    }

    QList<Quotient::EventContent::Answer> answerStructs;
    for (const auto &answer : answers) {
        answerStructs += Quotient::EventContent::Answer{
            QUuid::createUuid().toString().remove(QRegularExpression(u"{|}|-"_s)),
            answer,
        };
    }
    const auto content = Quotient::EventContent::PollStartContent{
        .kind = kind,
        .maxSelection = 1,
        .question = question,
        .answers = answerStructs,
    };
    m_room->post<Quotient::PollStartEvent>(content);
}

void PostMessageHelper::postVoiceMessage(QMediaRecorder *recorder)
{
    if (!m_room || !recorder) {
        return;
    }
    QPointer<QBuffer> buffer = dynamic_cast<QBuffer *>(recorder->outputDevice());
    if (!buffer) {
        return;
    }
    recorder->setOutputDevice(nullptr);

    Quotient::FileSourceInfo fileMetadata;
    buffer->seek(0);

    if (m_room->usesEncryption()) {
        QByteArray data;
        std::tie(fileMetadata, data) = Quotient::encryptFile(buffer->data());
        buffer->close();
        buffer->setData(data);
        buffer->open(QIODevice::ReadOnly);
    }

    auto room = m_room;
    const auto duration = recorder->duration();
    room->connection()->uploadContent(buffer, {}, u"audio/ogg"_s).then(this, [fileMetadata, room, buffer, duration](const auto &job) mutable {
        if (!room || !buffer) {
            return;
        }

        QJsonObject mscFile{
            {u"mimetype"_s, u"audio/ogg"_s},
            {u"name"_s, u"Voice Message"_s},
            {u"size"_s, buffer->size()},
        };

        if (room->usesEncryption()) {
            mscFile[u"file"_s] = toJson(fileMetadata);
        } else {
            mscFile[u"url"_s] = job->contentUri().toString();
        }

        Quotient::setUrlInSourceInfo(fileMetadata, job->contentUri());
        QJsonObject content{
            {u"body"_s, u"Voice message"_s},
            {u"msgtype"_s, u"m.audio"_s},
            {u"org.matrix.msc1767.text"_s,
             QJsonObject{{u"body"_s, u"Voice Message (%1, %2)"_s.arg(KFormat().formatDuration(duration), KFormat().formatByteSize(buffer->size()))}}},
            {u"org.matrix.msc1767.file"_s, mscFile},
            {u"info"_s,
             QJsonObject{
                 {u"mimetype"_s, u"audio/ogg"_s},
                 {u"size"_s, buffer->size()},
                 {u"duration"_s, duration},
             }},
            {u"org.matrix.msc1767.audio"_s,
             QJsonObject{
                 {u"duration"_s, duration},
                 {u"waveform"_s, QJsonArray{}}, // TODO
             }},
            {u"org.matrix.msc3245.voice"_s, QJsonObject{}}};
        if (room->usesEncryption()) {
            content[u"file"_s] = toJson(fileMetadata);
        } else {
            content[u"url"_s] = job->contentUri().toString();
        }
        room->postJson(u"m.room.message"_s, content);
    });
}

#include "moc_postmessagehelper.cpp"
