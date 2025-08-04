// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecomponenttype.h"

#include <QMimeDatabase>

#include <Quotient/events/encryptedevent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/stickerevent.h>

#include "events/pollevent.h"

const QList<MessageComponentType::Type> MessageComponentType::textTypes = {
    Text,
    Code,
    Quote,
};

const QList<MessageComponentType::Type> MessageComponentType::fileTypes = {
    File,
    Image,
    Video,
    Audio,
};

MessageComponentType::Type MessageComponentType::typeForEvent(const Quotient::RoomEvent &event, bool isInReply)
{
    using namespace Quotient;

    if (event.isRedacted()) {
        return MessageComponentType::Text;
    }

    if (const auto e = eventCast<const RoomMessageEvent>(&event)) {
        if (e->rawMsgtype() == u"m.key.verification.request"_s) {
            return MessageComponentType::Verification;
        }

        switch (e->msgtype()) {
        case MessageEventType::Emote:
            return MessageComponentType::Text;
        case MessageEventType::Notice:
            return MessageComponentType::Text;
        case MessageEventType::Image:
            return MessageComponentType::Image;
        case MessageEventType::Audio:
            return MessageComponentType::Audio;
        case MessageEventType::Video:
            return MessageComponentType::Video;
        case MessageEventType::Location:
            return MessageComponentType::Location;
        case MessageEventType::File:
            return MessageComponentType::File;
        default:
            return MessageComponentType::Text;
        }
    }
    if (is<const StickerEvent>(event)) {
        return MessageComponentType::Image;
    }
    if (event.isStateEvent()) {
        if (event.matrixType() == u"org.matrix.msc3672.beacon_info"_s) {
            return MessageComponentType::LiveLocation;
        }
        // In the (unlikely) case that this is a reply to a state event, we do want to show something
        return isInReply ? MessageComponentType::Text : MessageComponentType::Other;
    }
    if (is<const EncryptedEvent>(event)) {
        return MessageComponentType::Encrypted;
    }
    if (is<PollStartEvent>(event)) {
        const auto pollEvent = eventCast<const PollStartEvent>(&event);
        if (pollEvent->isRedacted()) {
            return MessageComponentType::Text;
        }
        return MessageComponentType::Poll;
    }

    // In the (unlikely) case that this is a reply to an unusual event, we do want to show something
    return isInReply ? MessageComponentType::Text : MessageComponentType::Other;
}

MessageComponentType::Type MessageComponentType::typeForString(const QString &string)
{
    if (string.isEmpty()) {
        return Text;
    }

    if (string.startsWith(u'>')) {
        return Quote;
    }
    if (string.startsWith(u"```"_s) && string.endsWith(u"```"_s)) {
        return Code;
    }

    return Text;
}

MessageComponentType::Type MessageComponentType::typeForTag(const QString &tag)
{
    if (tag == u"pre"_s || tag == u"pre"_s) {
        return Code;
    }
    if (tag == u"blockquote"_s) {
        return Quote;
    }
    return Text;
}

MessageComponentType::Type MessageComponentType::typeForPath(const QUrl &path)
{
    auto mime = QMimeDatabase().mimeTypeForUrl(path);
    if (mime.name().startsWith("image/"_L1)) {
        return Image;
    } else if (mime.name().startsWith("audio/"_L1)) {
        return Audio;
    } else if (mime.name().startsWith("video/"_L1)) {
        return Video;
    }
    return File;
}

bool MessageComponentType::isTextType(const MessageComponentType::Type &type)
{
    return textTypes.contains(type);
}

bool MessageComponentType::isFileType(const MessageComponentType::Type &type)
{
    return fileTypes.contains(type);
}
