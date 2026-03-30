// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "blocktype.h"

#include <QMimeDatabase>

#include <Quotient/events/encryptedevent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/stickerevent.h>

#include "events/pollevent.h"

Blocks::Type Blocks::typeForEvent(const Quotient::RoomEvent &event, bool isInReply)
{
    using namespace Quotient;

    if (event.isRedacted()) {
        return Text;
    }

    if (const auto e = eventCast<const RoomMessageEvent>(&event)) {
        if (e->rawMsgtype() == u"m.key.verification.request"_s) {
            return Verification;
        }

        switch (e->msgtype()) {
        case MessageEventType::Emote:
            return Text;
        case MessageEventType::Notice:
            return Text;
        case MessageEventType::Image:
            return Image;
        case MessageEventType::Audio:
            return Audio;
        case MessageEventType::Video:
            return Video;
        case MessageEventType::Location:
            return Location;
        case MessageEventType::File:
            return File;
        default:
            return Text;
        }
    }
    if (is<const StickerEvent>(event)) {
        return Image;
    }
    if (event.isStateEvent()) {
        if (event.matrixType() == u"org.matrix.msc3672.beacon_info"_s) {
            return LiveLocation;
        }
        // In the (unlikely) case that this is a reply to a state event, we do want to show something
        return isInReply ? Text : Other;
    }
    if (is<const EncryptedEvent>(event)) {
        return Encrypted;
    }
    if (is<PollStartEvent>(event)) {
        const auto pollEvent = eventCast<const PollStartEvent>(&event);
        if (pollEvent->isRedacted()) {
            return Text;
        }
        return Poll;
    }

    // In the (unlikely) case that this is a reply to an unusual event, we do want to show something
    return isInReply ? Text : Other;
}

Blocks::Type Blocks::typeForTag(const QString &tag)
{
    if (tag == u"pre"_s || tag == u"pre"_s) {
        return Code;
    }
    if (tag == u"blockquote"_s) {
        return Quote;
    }
    return Text;
}

Blocks::Type Blocks::typeForPath(const QUrl &path)
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

bool Blocks::isTextType(const Blocks::Type &type)
{
    static const QList<Type> textTypes = {
        Text,
        Code,
        Quote,
    };
    return textTypes.contains(type);
}

bool Blocks::isFileType(const Blocks::Type &type)
{
    static const QList<Type> fileTypes = {
        File,
        Image,
        Video,
        Audio,
    };
    return fileTypes.contains(type);
}

#include "moc_blocktype.cpp"
