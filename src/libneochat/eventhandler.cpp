// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "eventhandler.h"

#include <QMovie>

#include <KLocalizedString>

#include <Quotient/events/encryptionevent.h>
#include <Quotient/events/event.h>
#include <Quotient/events/eventcontent.h>
#include <Quotient/events/reactionevent.h>
#include <Quotient/events/redactionevent.h>
#include <Quotient/events/roomavatarevent.h>
#include <Quotient/events/roomcanonicalaliasevent.h>
#include <Quotient/events/roomevent.h>
#include <Quotient/events/roommemberevent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/roompowerlevelsevent.h>
#include <Quotient/events/simplestateevents.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/quotient_common.h>
#include <Quotient/roommember.h>
#include <Quotient/thread.h>

#include "block.h"
#include "blocktype.h"
#include "eventhandler_logging.h"
#include "events/locationbeaconevent.h"
#include "events/pollevent.h"
#include "events/widgetevent.h"
#include "fileinfo.h"
#include "neochatroom.h"
#include "pollblock.h"
#include "texthandler.h"
#include "utils.h"

using namespace Quotient;

namespace
{
enum MemberChange {
    None = 0,
    AddName = 1,
    Rename = 2,
    RemoveName = 4,
    AddAvatar = 8,
    UpdateAvatar = 16,
    RemoveAvatar = 32,
};
Q_DECLARE_FLAGS(MemberChanges, MemberChange)
Q_DECLARE_OPERATORS_FOR_FLAGS(MemberChanges)
};

QString EventHandler::authorDisplayName(const NeoChatRoom *room, const RoomEvent *event, bool isPending)
{
    if (!room) {
        qCWarning(EventHandling) << "authorDisplayName called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << "authorDisplayName called with event set to nullptr.";
        return {};
    }

    if (is<RoomMemberEvent>(*event) && event->unsignedJson()["prev_content"_L1].toObject().contains("displayname"_L1)
        && event->stateKey() == event->senderId()) {
        if (auto previousDisplayName = event->unsignedJson()["prev_content"_L1]["displayname"_L1].toString().toHtmlEscaped(); !previousDisplayName.isEmpty()) {
            return previousDisplayName;
        }
        return event->senderId();
    }

    const auto author = isPending ? room->localMember() : room->member(event->senderId());
    return author.htmlSafeDisplayName();
}

QString EventHandler::singleLineAuthorDisplayName(const NeoChatRoom *room, const RoomEvent *event, bool isPending)
{
    if (!room) {
        qCWarning(EventHandling) << "singleLineAuthorDisplayName called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << "singleLineAuthorDisplayName called with event set to nullptr.";
        return {};
    }

    const auto author = isPending ? room->localMember() : room->member(event->senderId());
    auto displayName = author.displayName();
    displayName.replace(u"<br>\n"_s, u" "_s);
    displayName.replace(u"<br>"_s, u" "_s);
    displayName.replace(u"<br />\n"_s, u" "_s);
    displayName.replace(u"<br />"_s, u" "_s);
    displayName.replace(u'\n', u" "_s);
    displayName.replace(u'\u2028', u" "_s);
    return displayName;
}

NeoChatDateTime EventHandler::dateTime(const NeoChatRoom *room, const RoomEvent *event, bool isPending)
{
    if (!room) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with event set to nullptr.";
        return {};
    }

    if (isPending) {
        if (const auto pendingIt = room->findPendingEvent(event->transactionId()); pendingIt != room->pendingEvents().end()) {
            return pendingIt->lastUpdated();
        }
        return {};
    }
    return event->originTimestamp();
}

bool EventHandler::isHighlighted(const NeoChatRoom *room, const RoomEvent *event)
{
    if (!room) {
        qCWarning(EventHandling) << "isHighlighted called with room set to nullptr.";
        return false;
    }
    if (!event) {
        qCWarning(EventHandling) << "isHighlighted called with event set to nullptr.";
        return false;
    }

    return !room->isDirectChat() && room->isEventHighlighted(event);
}

bool EventHandler::isHidden(const NeoChatRoom *room, const RoomEvent *event, const std::function<bool(const RoomEvent *)> &filter)
{
    if (!room) {
        qCWarning(EventHandling) << "isHidden called with room set to nullptr.";
        return false;
    }
    if (!event) {
        qCWarning(EventHandling) << "isHidden called with event set to nullptr.";
        return false;
    }

    if (filter && filter(event)) {
        return true;
    }

    if (event->isStateEvent() && eventCast<const StateEvent>(event)->repeatsState()) {
        return true;
    }

    // isReplacement?
    if (const auto roomMessageEvent = eventCast<const RoomMessageEvent>(event)) {
        if (!roomMessageEvent->replacedEvent().isEmpty()) {
            return true;
        }
    }

    if (is<RedactionEvent>(*event) || is<ReactionEvent>(*event)) {
        return true;
    }

    if (auto roomMessageEvent = eventCast<const RoomMessageEvent>(event)) {
        if (!roomMessageEvent->replacedEvent().isEmpty() && roomMessageEvent->replacedEvent() != roomMessageEvent->id()) {
            return true;
        }
    }

    if (room->connection()->isIgnored(event->senderId())) {
        return true;
    }

    // hide ending live location beacons
    if (event->isStateEvent() && event->matrixType() == "org.matrix.msc3672.beacon_info"_L1 && !event->contentPart<bool>("live"_L1)) {
        return true;
    }

    return false;
}

Qt::TextFormat EventHandler::messageBodyInputFormat(const RoomEvent &event)
{
    if (event.isRedacted() && !event.isStateEvent()) {
        return Qt::RichText;
    }

    const auto msgEvent = eventCast<const RoomMessageEvent>(&event);
    if (!msgEvent) {
        return Qt::PlainText;
    }

    if (msgEvent->mimeType().name() == "text/plain"_L1) {
        return Qt::PlainText;
    }
    return Qt::RichText;
}

QString EventHandler::rawMessageBody(const RoomEvent &event)
{
    if (event.isRedacted() && !event.isStateEvent()) {
        const auto reason = event.redactedBecause()->reason();
        return reason.isEmpty() ? i18n("<i>[This message was deleted]</i>") : i18n("<i>[This message was deleted: %1]</i>", reason.toHtmlEscaped());
    }

    const auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event);
    if (!roomMessageEvent) {
        return {};
    }

    if (roomMessageEvent->has<EventContent::FileContent>()) {
        // if filename is given or body is equal to filename,
        // then body is a caption
        const auto filename = roomMessageEvent->get<EventContent::FileContent>()->originalName;
        auto body = roomMessageEvent->plainBody();
        if (filename.isEmpty() || filename == body) {
            return {};
        }
        return body;
    }

    if (roomMessageEvent->has<EventContent::TextContent>() && roomMessageEvent->content()) {
        return roomMessageEvent->get<EventContent::TextContent>()->body;
    }
    return roomMessageEvent->plainBody();
}

QString EventHandler::richBody(const NeoChatRoom *room, const RoomEvent *event, bool stripNewlines)
{
    if (!room) {
        qCWarning(EventHandling) << "richBody called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << "richBody called with event set to nullptr.";
        return {};
    }
    return getBody(room, event, Qt::RichText, stripNewlines);
}

QString EventHandler::plainBody(const NeoChatRoom *room, const RoomEvent *event, bool stripNewlines)
{
    if (!room) {
        qCWarning(EventHandling) << "plainBody called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << "plainBody called with event set to nullptr.";
        return {};
    }
    return getBody(room, event, Qt::PlainText, stripNewlines);
}

QString EventHandler::markdownBody(const RoomEvent *event)
{
    if (!event) {
        qCWarning(EventHandling) << "markdownBody called with event set to nullptr.";
        return {};
    }

    if (!event->is<RoomMessageEvent>()) {
        qCWarning(EventHandling) << "markdownBody called when event isn't a RoomMessageEvent.";
        return {};
    }

    auto plainBody = eventCast<const RoomMessageEvent>(event)->plainBody();
    plainBody.remove(TextRegex::removeReply);
    return plainBody;
}

QString EventHandler::getBody(const NeoChatRoom *room, const RoomEvent *event, Qt::TextFormat format, bool stripNewlines)
{
    if (!room) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with event set to nullptr.";
        return {};
    }

    if (event->isRedacted() && !event->isStateEvent()) {
        const auto reason = event->redactedBecause()->reason();
        return reason.isEmpty() ? i18n("<i>[This message was deleted]</i>") : i18n("<i>[This message was deleted: %1]</i>", reason.toHtmlEscaped());
    }

    const bool prettyPrint = format == Qt::RichText;

    return switchOnType(
        *event,
        [room, format, stripNewlines](const RoomMessageEvent &roomMessageEvent) {
            return getMessageBody(room, roomMessageEvent, format, stripNewlines);
        },
        [](const StickerEvent &e) {
            return e.body();
        },
        [room, prettyPrint](const RoomMemberEvent &e) {
            // FIXME: Rewind to the name that was at the time of this event
            auto subjectName = prettyPrint ? room->member(e.userId()).htmlSafeDisplayName() : room->member(e.userId()).displayName();
            if (e.membership() == Membership::Leave) {
                if (e.prevContent() && e.prevContent()->displayName) {
                    subjectName = sanitized(*e.prevContent()->displayName);
                    if (prettyPrint) {
                        subjectName = subjectName.toHtmlEscaped();
                    }
                }
            }

            if (prettyPrint) {
                subjectName =
                    u"<a href=\"https://matrix.to/#/%1\" style=\"color: %2\">%3</a>"_s.arg(e.userId(), room->member(e.userId()).color().name(), subjectName);
            }

            // The below code assumes senderName output in AuthorRole
            switch (e.membership()) {
            case Membership::Invite:
                if (e.repeatsState()) {
                    auto text = i18n("reinvited %1 to the room", subjectName);
                    if (!e.reason().isEmpty()) {
                        text += i18nc("Optional reason for an invitation", ": %1") + (prettyPrint ? e.reason().toHtmlEscaped() : e.reason());
                    }
                    return text;
                }
                Q_FALLTHROUGH();
            case Membership::Join: {
                QString text{};
                // Part 1: invites and joins
                if (e.repeatsState()) {
                    text = i18n("joined the room (repeated)");
                } else if (e.changesMembership()) {
                    text = e.membership() == Membership::Invite ? i18n("invited %1 to the room", subjectName) : i18n("joined the room");
                }
                if (!text.isEmpty()) {
                    if (!e.reason().isEmpty()) {
                        text += i18n(": %1", e.reason().toHtmlEscaped());
                    }
                    return text;
                }
                // Part 2: profile changes of joined members
                if (e.isRename()) {
                    if (!e.newDisplayName()) {
                        text = i18nc("their refers to a singular user", "cleared their display name");
                    } else {
                        text = i18nc("their refers to a singular user",
                                     "changed their display name to %1",
                                     prettyPrint ? e.newDisplayName()->toHtmlEscaped() : *e.newDisplayName());
                    }
                }
                if (e.isAvatarUpdate()) {
                    if (!text.isEmpty()) {
                        text += i18n(" and ");
                    }
                    if (!e.newAvatarUrl()) {
                        text += i18nc("their refers to a singular user", "cleared their avatar");
                    } else if (!e.prevContent()->avatarUrl) {
                        text += i18n("set an avatar");
                    } else {
                        text += i18nc("their refers to a singular user", "updated their avatar");
                    }
                }
                if (text.isEmpty()) {
                    text = i18nc("<user> changed nothing", "changed nothing");
                }
                return text;
            }
            case Membership::Leave:
                if (e.prevContent() && e.prevContent()->membership == Membership::Invite) {
                    return (e.senderId() != e.userId()) ? i18n("withdrew %1's invitation", subjectName) : i18n("rejected the invitation");
                }

                if (e.prevContent() && e.prevContent()->membership == Membership::Ban) {
                    return (e.senderId() != e.userId()) ? i18n("unbanned %1", subjectName) : i18n("self-unbanned");
                }
                if (e.senderId() == e.userId()) {
                    return i18n("left the room");
                }
                if (const auto &reason = e.contentPart<QString>("reason"_L1).toHtmlEscaped(); !reason.isEmpty()) {
                    return i18n("has removed %1 from the room: %2", subjectName, reason);
                }
                return i18n("has removed %1 from the room", subjectName);
            case Membership::Ban:
                if (e.senderId() != e.userId()) {
                    if (e.reason().isEmpty()) {
                        return i18n("banned %1 from the room", subjectName);
                    } else {
                        return i18n("banned %1 from the room: %2", subjectName, prettyPrint ? e.reason().toHtmlEscaped() : e.reason());
                    }
                } else {
                    return i18n("self-banned from the room");
                }
            case Membership::Knock: {
                QString reason(e.contentPart<QString>("reason"_L1).toHtmlEscaped());
                return reason.isEmpty() ? i18n("requested an invite") : i18n("requested an invite with reason: %1", reason);
            }
            default:;
            }
            return i18n("made something unknown");
        },
        [](const RoomCanonicalAliasEvent &e) {
            return e.alias().isEmpty() ? i18n("cleared the room main alias") : i18n("set the room main alias to: %1", e.alias());
        },
        [prettyPrint](const RoomNameEvent &e) {
            return e.name().isEmpty() ? i18n("cleared the room name") : i18n("set the room name to: %1", prettyPrint ? e.name().toHtmlEscaped() : e.name());
        },
        [prettyPrint, stripNewlines](const RoomTopicEvent &e) {
            return (e.topic().isEmpty()) ? i18n("cleared the topic")
                                         : i18n("set the topic to: %1",
                                                prettyPrint         ? Quotient::prettyPrint(e.topic())
                                                    : stripNewlines ? e.topic().replace(u'\n', u' ')
                                                                    : e.topic());
        },
        [](const RoomAvatarEvent &) {
            return i18n("changed the room avatar");
        },
        [](const EncryptionEvent &) {
            return i18n("activated End-to-End Encryption");
        },
        [prettyPrint](const RoomCreateEvent &e) {
            return e.isUpgrade()
                ? i18n("upgraded the room to version %1", e.version().isEmpty() ? "1"_L1 : (prettyPrint ? e.version().toHtmlEscaped() : e.version()))
                : i18n("created the room, version %1", e.version().isEmpty() ? "1"_L1 : (prettyPrint ? e.version().toHtmlEscaped() : e.version()));
        },
        [](const RoomPowerLevelsEvent &) {
            return i18nc("'power level' means permission level", "changed the power levels for this room");
        },
        [](const LocationBeaconEvent &e) {
            return e.contentPart<QString>("description"_L1);
        },
        [](const RoomServerAclEvent &) {
            return i18n("changed the server access control lists for this room");
        },
        [](const WidgetEvent &e) {
            if (e.fullJson()["unsigned"_L1]["prev_content"_L1].toObject().isEmpty()) {
                return i18nc("[User] added <name> widget", "added %1 widget", e.contentPart<QString>("name"_L1));
            }
            if (e.contentJson().isEmpty()) {
                return i18nc("[User] removed <name> widget", "removed %1 widget", e.fullJson()["unsigned"_L1]["prev_content"_L1]["name"_L1].toString());
            }
            return i18nc("[User] configured <name> widget", "configured %1 widget", e.contentPart<QString>("name"_L1));
        },
        [prettyPrint](const StateEvent &e) {
            if (e.matrixType() == "org.matrix.msc3401.call.member"_L1) {
                if (e.contentJson().isEmpty()) {
                    return i18nc("[User] left a [voice/video] call", "left a call");
                }
                return i18nc("[User] joined a [voice/video] call", "joined a call");
            }
            if (e.matrixType() == "io.element.integrations.installations"_L1) {
                return i18nc("[User] configured an extension", "configured an extension");
            }
            return e.stateKey().isEmpty() ? i18n("updated %1 state", e.matrixType())
                                          : i18n("updated %1 state for %2", e.matrixType(), prettyPrint ? e.stateKey().toHtmlEscaped() : e.stateKey());
        },
        [](const PollStartEvent &e) {
            return e.question();
        },
        [](const EncryptedEvent &) {
            return i18nc("@info In room list", "Encrypted event");
        },
        [](const ReactionEvent &e) {
            return i18nc("[user] reacted with <emoji>", "reacted with %1", e.key());
        },
        i18n("Unknown event"));
}

QString EventHandler::getMessageBody(const NeoChatRoom *room, const RoomMessageEvent &event, Qt::TextFormat format, bool stripNewlines)
{
    if (!room) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with room set to nullptr.";
        return {};
    }

    TextHandler textHandler;

    if (event.has<EventContent::FileContent>()) {
        QString fileCaption = event.get<EventContent::FileContent>()->originalName;
        if (fileCaption.isEmpty()) {
            fileCaption = event.plainBody();
        } else if (fileCaption != event.plainBody()) {
            fileCaption = event.plainBody() + " | "_L1 + fileCaption;
        }
        textHandler.setData(fileCaption);
        return !fileCaption.isEmpty() ? textHandler.handleReceivePlainText(Qt::PlainText, stripNewlines) : i18n("a file");
    }

    QString body;
    if (event.has<EventContent::TextContent>() && event.content()) {
        body = event.get<EventContent::TextContent>()->body;
    } else {
        body = event.plainBody();
    }

    textHandler.setData(body);

    Qt::TextFormat inputFormat;
    if (event.mimeType().name() == "text/plain"_L1) {
        inputFormat = Qt::PlainText;
    } else {
        inputFormat = Qt::RichText;
    }

    if (format == Qt::RichText) {
        return textHandler.handleReceiveRichText(inputFormat, room, &event, stripNewlines, event.isReplaced());
    }
    return textHandler.handleReceivePlainText(inputFormat, stripNewlines);
}

QString EventHandler::genericBody(const NeoChatRoom *room, const RoomEvent *event)
{
    if (!room) {
        qCWarning(EventHandling) << "genericBody called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << "genericBody called with event set to nullptr.";
        return {};
    }
    if (event->isRedacted() && !event->isStateEvent()) {
        return i18n("<i>[This message was deleted]</i>");
    }

    const auto sender = room->member(event->senderId());
    const auto senderString = u"<a href=\"https://matrix.to/#/%1\">%2</a>"_s.arg(sender.id(), sender.htmlSafeDisplayName());

    return switchOnType(
        *event,
        [senderString](const RoomMessageEvent &) {
            return i18n("%1 sent a message", senderString);
        },
        [senderString](const StickerEvent &) {
            return i18n("%1 sent a sticker", senderString);
        },
        [senderString](const RoomMemberEvent &e) {
            switch (e.membership()) {
            case Membership::Invite:
                if (e.repeatsState()) {
                    return i18n("%1 reinvited someone to the room", senderString);
                }
                Q_FALLTHROUGH();
            case Membership::Join: {
                // Part 1: invites and joins
                if (e.repeatsState()) {
                    return i18n("%1 joined the room (repeated)", senderString);
                }
                if (e.changesMembership()) {
                    return e.membership() == Membership::Invite ? i18n("%1 invited someone to the room", senderString)
                                                                : i18n("%1 joined the room", senderString);
                }

                // Part 2: profile changes of joined members
                MemberChanges changes = None;
                if (e.isRename()) {
                    if (!e.newDisplayName()) {
                        changes |= RemoveName;
                    } else if (!e.prevContent()->displayName) {
                        changes |= AddName;
                    } else {
                        changes |= Rename;
                    }
                }
                if (e.isAvatarUpdate()) {
                    if (!e.newAvatarUrl()) {
                        changes |= RemoveAvatar;
                    } else if (!e.prevContent()->avatarUrl) {
                        changes |= AddAvatar;
                    } else {
                        changes |= UpdateAvatar;
                    }
                }

                if (changes.testFlag(AddName)) {
                    if (changes.testFlag(AddAvatar)) {
                        return i18n("%1 set a display name and set an avatar", senderString);
                    }
                    if (changes.testFlag(UpdateAvatar)) {
                        return i18n("%1 set a display name and updated their avatar", senderString);
                    }
                    if (changes.testFlag(RemoveAvatar)) {
                        return i18n("%1 set a display name and cleared their avatar", senderString);
                    }
                    return i18n("%1 set a display name for this room", senderString);
                }
                if (changes.testFlag(Rename)) {
                    if (changes.testFlag(AddAvatar)) {
                        return i18n("%1 changed their display name and set an avatar", senderString);
                    }
                    if (changes.testFlag(UpdateAvatar)) {
                        return i18n("%1 changed their display name and updated their avatar", senderString);
                    }
                    if (changes.testFlag(RemoveAvatar)) {
                        return i18n("%1 changed their display name and cleared their avatar", senderString);
                    }
                    return i18n("%1 changed their display name", senderString);
                }
                if (changes.testFlag(RemoveName)) {
                    if (changes.testFlag(AddAvatar)) {
                        return i18n("%1 cleared their display name and set an avatar", senderString);
                    }
                    if (changes.testFlag(UpdateAvatar)) {
                        return i18n("%1 cleared their display name and updated their avatar", senderString);
                    }
                    if (changes.testFlag(RemoveAvatar)) {
                        return i18n("%1 cleared their display name and cleared their avatar", senderString);
                    }
                    return i18n("%1 cleared their display name", senderString);
                }

                return i18nc("<user> changed nothing", "%1 changed nothing", senderString);
            }
            case Membership::Leave:
                if (e.prevContent() && e.prevContent()->membership == Membership::Invite) {
                    return e.senderId() != e.userId() ? i18n("%1 withdrew a user's invitation", senderString)
                                                      : i18n("%1 rejected the invitation", senderString);
                }

                if (e.prevContent() && e.prevContent()->membership == Membership::Ban) {
                    return e.senderId() != e.userId() ? i18n("%1 unbanned a user", senderString) : i18n("%1 self-unbanned", senderString);
                }
                return e.senderId() != e.userId() ? i18n("%1 put a user out of the room", senderString) : i18n("%1 left the room", senderString);
            case Membership::Ban: {
                if (e.senderId() != e.userId()) {
                    return i18n("%1 banned a user from the room", senderString);
                }
                return i18n("%1 self-banned from the room", senderString);
            }
            case Membership::Knock: {
                return i18n("%1 requested an invite", senderString);
            }
            default:;
            }
            return i18n("%1 made something unknown", senderString);
        },
        [senderString](const RoomCanonicalAliasEvent &e) {
            return e.alias().isEmpty() ? i18n("%1 cleared the room main alias", senderString) : i18n("%1 set the room main alias", senderString);
        },
        [senderString](const RoomNameEvent &e) {
            return e.name().isEmpty() ? i18n("%1 cleared the room name", senderString) : i18n("%1 set the room name", senderString);
        },
        [senderString](const RoomTopicEvent &e) {
            return e.topic().isEmpty() ? i18n("%1 cleared the topic", senderString) : i18n("%1 set the topic", senderString);
        },
        [senderString](const RoomAvatarEvent &) {
            return i18n("%1 changed the room avatar", senderString);
        },
        [senderString](const EncryptionEvent &) {
            return i18n("%1 activated End-to-End Encryption", senderString);
        },
        [senderString](const RoomCreateEvent &e) {
            return e.isUpgrade() ? i18n("%1 upgraded the room version", senderString) : i18n("%1 created the room", senderString);
        },
        [senderString](const RoomPowerLevelsEvent &) {
            return i18nc("'power level' means permission level", "%1 changed the power levels for this room", senderString);
        },
        [senderString](const LocationBeaconEvent &) {
            return i18n("%1 sent a live location beacon", senderString);
        },
        [senderString](const RoomServerAclEvent &) {
            return i18n("%1 changed the server access control lists for this room", senderString);
        },
        [senderString](const WidgetEvent &e) {
            if (e.fullJson()["unsigned"_L1]["prev_content"_L1].toObject().isEmpty()) {
                return i18n("%1 added a widget", senderString);
            }
            if (e.contentJson().isEmpty()) {
                return i18n("%1 removed a widget", senderString);
            }
            return i18n("%1 configured a widget", senderString);
        },
        [senderString](const StateEvent &e) {
            if (e.matrixType() == "org.matrix.msc3401.call.member"_L1) {
                if (e.contentJson().isEmpty()) {
                    return i18nc("[User] left a [voice/video] call", "%1 left a call", senderString);
                }
                return i18nc("[User] joined a [voice/video] call", "%1 joined a call", senderString);
            }
            if (e.matrixType() == "io.element.integrations.installations"_L1) {
                return i18nc("[User] configured an extension", "%1 configured an extension", senderString);
            }
            return i18n("%1 updated the state", senderString);
        },
        [senderString](const PollStartEvent &) {
            return i18n("%1 started a poll", senderString);
        },
        i18n("Unknown event"));
}

QString EventHandler::subtitleText(const NeoChatRoom *room, const RoomEvent *event)
{
    if (!room) {
        qCWarning(EventHandling) << "subtitleText called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << "subtitleText called with event set to nullptr.";
        return {};
    }
    if (room->isDirectChat()) {
        return plainBody(room, event, true);
    }
    return singleLineAuthorDisplayName(room, event) + (event->isStateEvent() ? u" "_s : u": "_s) + plainBody(room, event, true);
}

bool EventHandler::isMediaMessage(const RoomEvent *event)
{
    if (!event) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with event set to nullptr.";
        return {};
    }
    if (!event->is<RoomMessageEvent>()) {
        return false;
    }
    const auto roomMessageEvent = eventCast<const RoomMessageEvent>(event);
    return roomMessageEvent->has<EventContent::ImageContent>() || roomMessageEvent->has<EventContent::VideoContent>();
}

Blocks::BlockPtrs EventHandler::blocksForEvent(NeoChatRoom *room, const RoomEvent *event, QObject *parent)
{
    if (!room) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with event set to nullptr.";
        return {};
    }
    if (!parent) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with parent set to nullptr.";
        return {};
    }
    Blocks::BlockPtrs blocks;
    blocks.insert_range(blocks.end(), blocksForEventType(room, event, parent));

    const auto roomMessageEvent = eventCast<const RoomMessageEvent>(event);
    if (roomMessageEvent
        && ((roomMessageEvent->isThreaded() && roomMessageEvent->id() == roomMessageEvent->threadRootEventId())
            || room->threads().contains(roomMessageEvent->id()))) {
        blocks.push_back(new Blocks::Block(Blocks::Separator, parent));
        blocks.push_back(new Blocks::Block(Blocks::ThreadBody, parent));
    }

    return blocks;
}

Blocks::BlockPtrs EventHandler::blocksForEventType(NeoChatRoom *room, const RoomEvent *event, QObject *parent)
{
    if (!room) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with event set to nullptr.";
        return {};
    }
    if (!parent) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with parent set to nullptr.";
        return {};
    }
#if Quotient_VERSION_MINOR > 9
    Blocks::Type type = Blocks::typeForEvent(*event, event->isReply());
#else
    const auto roomMessageEvent = eventCast<const RoomMessageEvent>(event);
    if (!roomMessageEvent) {
        return {};
    }
    Blocks::Type type = Blocks::typeForEvent(*roomMessageEvent, roomMessageEvent->isReply());
#endif
    switch (type) {
    case Blocks::Text: {
        return TextHandler().textComponents(rawMessageBody(*event),
                                            messageBodyInputFormat(*event),
                                            room,
                                            event,
#if Quotient_VERSION_MINOR > 9
                                            event->isReplaced(),
#else
                                            roomMessageEvent->isReplaced(),
#endif
                                            false,
                                            parent);
    }
    case Blocks::File:
    case Blocks::Image:
    case Blocks::Audio:
    case Blocks::Video: {
        Blocks::BlockPtrs components;
        components.push_back(blockForMediaEvent(room, event, parent));
        if (const auto body = rawMessageBody(*event); !event->is<StickerEvent>() && !body.isEmpty()) {
            components.insert_range(components.end(),
                                    TextHandler().textComponents(body,
                                                                 messageBodyInputFormat(*event),
                                                                 room,
                                                                 event,
#if Quotient_VERSION_MINOR > 9
                                                                 event->isReplaced(),
#else
                                                                 roomMessageEvent->isReplaced(),
#endif
                                                                 false,
                                                                 parent));
        }
        return components;
    }
    case Blocks::Location: {
        Blocks::BlockPtrs components;
        components.push_back(new Blocks::LocationBlock(type, latitude(event), longitude(event), locationAssetType(event), parent));
        components.push_back(new Blocks::TextBlock(Blocks::Text, QTextDocumentFragment::fromPlainText(plainBody(room, event)), false, parent));
        return components;
    }
    case Blocks::Poll: {
        Blocks::BlockPtrs blocks;
        blocks.push_back(new Blocks::PollBlock(type, event->id(), room, parent));
        return blocks;
    }
    case Blocks::Encrypted: {
        Blocks::BlockPtrs blocks;
        blocks.push_back(new Blocks::Block(Blocks::Encrypted, parent));
        return blocks;
    }
    default:
        return {};
    }
}

Blocks::Block *EventHandler::blockForMediaEvent(NeoChatRoom *room, const RoomEvent *event, QObject *parent)
{
    if (!room) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with room set to nullptr.";
        return nullptr;
    }
    if (!event) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with event set to nullptr.";
        return nullptr;
    }

    const auto eventId = event->id();

    // Get the file info for the event.
    if (event->is<RoomMessageEvent>()) {
        const auto roomMessageEvent = eventCast<const RoomMessageEvent>(event);
        if (!roomMessageEvent->has<EventContent::FileContentBase>()) {
            return {};
        }

        const auto content = roomMessageEvent->get<EventContent::FileContentBase>();
        // if filename isn't specifically given, it is in body
        // https://spec.matrix.org/latest/client-server-api/#mfile
        const auto filename = content->commonInfo().originalName.isEmpty() ? roomMessageEvent->plainBody() : content->commonInfo().originalName;
        return fileBlockFromFileContent(parent, room, content.get(), eventId, filename, false);
    }
    if (event->is<StickerEvent>()) {
        const auto stickerEvent = eventCast<const StickerEvent>(event);
        const auto content = &stickerEvent->image();

        return fileBlockFromFileContent(parent, room, content, eventId, {}, true);
    }
    return {};
}

Blocks::Block *EventHandler::fileBlockFromFileContent(QObject *parent,
                                                      NeoChatRoom *room,
                                                      const EventContent::FileContentBase *fileContent,
                                                      const QString &eventId,
                                                      const QString &filename,
                                                      bool isSticker)
{
    if (!room) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with room set to nullptr.";
        return nullptr;
    }
    if (!fileContent) {
        qCWarning(EventHandling) << __FUNCTION__ << "called with fileContent set to nullptr.";
        return nullptr;
    }
    // Get the mxc URL for the media.
    QUrl source;
    if (fileContent->url().isValid() && fileContent->url().scheme() == u"mxc"_s && !eventId.isEmpty()) {
        source = room->makeMediaUrl(eventId, fileContent->url());
        if (!source.isValid()) {
            source = QUrl();
        }
    }

    const auto mimeType = fileContent->type();

    // Add parameter depending on media type.
    if (const auto videoContent = dynamic_cast<const EventContent::VideoContent *>(fileContent)) {
        Blocks::VideoInfo videoInfo;
        videoInfo.mimeType = mimeType;
        videoInfo.size = videoContent->payloadSize;
        videoInfo.pixelSize = videoContent->imageSize;
        videoInfo.duration = videoContent->duration;

        QUrl thumbnailSource;
        if (const auto thumbnail = videoContent->thumbnail; thumbnail.url().isValid() && thumbnail.url().scheme() == u"mxc"_s && !eventId.isEmpty()) {
            thumbnailSource = room->makeMediaUrl(eventId, thumbnail.url());
        } else {
            if (const auto blurhash = videoContent->originalInfoJson["xyz.amorgan.blurhash"_L1].toString(); !blurhash.isEmpty()) {
                thumbnailSource = QUrl("image://blurhash/"_L1 + blurhash);
            }
        }
        const auto thumbnailInfo = getThumbnailInfo(videoContent->thumbnail);
        return new Blocks::VideoBlock(Blocks::Video, source, filename, videoInfo, thumbnailSource, thumbnailInfo, room, eventId, parent);
    }
    if (const auto imageContent = dynamic_cast<const EventContent::ImageContent *>(fileContent)) {
        Blocks::ImageInfo imageInfo;
        imageInfo.mimeType = mimeType;
        imageInfo.size = imageContent->payloadSize;
        imageInfo.pixelSize = imageContent->imageSize;

        // TODO: Images in certain formats (e.g. WebP) will be erroneously marked as animated, even if they are static.
        imageInfo.isAnimated = QMovie::supportedFormats().contains(mimeType.preferredSuffix().toUtf8());
        imageInfo.isSticker = isSticker;

        QUrl thumbnailSource;
        if (const auto thumbnail = imageContent->thumbnail; thumbnail.url().isValid() && thumbnail.url().scheme() == u"mxc"_s && !eventId.isEmpty()) {
            thumbnailSource = room->makeMediaUrl(eventId, thumbnail.url());
        } else {
            if (const auto blurhash = imageContent->originalInfoJson["xyz.amorgan.blurhash"_L1].toString(); !blurhash.isEmpty()) {
                thumbnailSource = QUrl("image://blurhash/"_L1 + blurhash);
            }
        }
        const auto thumbnailInfo = getThumbnailInfo(imageContent->thumbnail);
        return new Blocks::ImageBlock(Blocks::Image, source, filename, imageInfo, thumbnailSource, thumbnailInfo, true, parent);
    }
    if (const auto audioContent = dynamic_cast<const EventContent::AudioContent *>(fileContent)) {
        Blocks::AudioInfo audioInfo;
        audioInfo.mimeType = mimeType;
        audioInfo.size = audioContent->payloadSize;
        audioInfo.duration = audioContent->duration;
        return new Blocks::AudioBlock(Blocks::Audio, source, filename, audioInfo, room, eventId, parent);
    }

    Blocks::FileInfo info;
    info.mimeType = mimeType;
    info.size = fileContent->commonInfo().payloadSize;
    return new Blocks::FileBlock(Blocks::File, source, filename, info, room, eventId, parent);
}

Blocks::ImageInfo EventHandler::getThumbnailInfo(const EventContent::Thumbnail &thumbnail)
{
    Blocks::ImageInfo thumbnailInfo;
    thumbnailInfo.mimeType = thumbnail.mimeType;
    thumbnailInfo.size = thumbnail.payloadSize;
    thumbnailInfo.pixelSize = thumbnail.imageSize;
    return thumbnailInfo;
}

RoomMember EventHandler::replyAuthor(const NeoChatRoom *room, const RoomEvent *event)
{
    if (!room) {
        qCWarning(EventHandling) << "replyAuthor called with room set to nullptr.";
        return {};
    }
    if (!event) {
        qCWarning(EventHandling) << "replyAuthor called with event set to nullptr. Returning empty user.";
        return {};
    }

    if (const auto replyPtr = room->getReplyForEvent(*event)) {
        return room->member(replyPtr->senderId());
    }
    return room->member(QString());
}

float EventHandler::latitude(const RoomEvent *event)
{
    if (!event) {
        qCWarning(EventHandling) << "latitude called with event set to nullptr.";
        return -100.0;
    }

    const auto geoUri = event->contentPart<QString>("geo_uri"_L1);
    if (geoUri.isEmpty()) {
        return -100.0; // latitude runs from -90deg to +90deg so -100 is out of range.
    }
    const auto latitude = geoUri.split(u';')[0].split(u':')[1].split(u',')[0];
    return latitude.toFloat();
}

float EventHandler::longitude(const RoomEvent *event)
{
    if (!event) {
        qCWarning(EventHandling) << "longitude called with event set to nullptr.";
        return -200.0;
    }

    const auto geoUri = event->contentPart<QString>("geo_uri"_L1);
    if (geoUri.isEmpty()) {
        return -200.0; // longitude runs from -180deg to +180deg so -200 is out of range.
    }
    const auto latitude = geoUri.split(u';')[0].split(u':')[1].split(u',')[1];
    return latitude.toFloat();
}

QString EventHandler::locationAssetType(const RoomEvent *event)
{
    if (!event) {
        qCWarning(EventHandling) << "locationAssetType called with event set to nullptr.";
        return {};
    }

    if (auto assetType = event->contentJson()["org.matrix.msc3488.asset"_L1]["type"_L1].toString(); !assetType.isEmpty()) {
        return assetType;
    }
    return {};
}

#include "moc_eventhandler.cpp"
