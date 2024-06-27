// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "eventhandler.h"

#include <QMovie>

#include <KLocalizedString>

#include <Quotient/eventitem.h>
#include <Quotient/events/encryptionevent.h>
#include <Quotient/events/reactionevent.h>
#include <Quotient/events/redactionevent.h>
#include <Quotient/events/roomavatarevent.h>
#include <Quotient/events/roomcanonicalaliasevent.h>
#include <Quotient/events/roommemberevent.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/events/roompowerlevelsevent.h>
#include <Quotient/events/simplestateevents.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/quotient_common.h>

#include "eventhandler_logging.h"
#include "events/locationbeaconevent.h"
#include "events/pollevent.h"
#include "events/serveraclevent.h"
#include "events/widgetevent.h"
#include "linkpreviewer.h"
#include "messagecomponenttype.h"
#include "models/reactionmodel.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include "texthandler.h"
#include "utils.h"

using namespace Quotient;

EventHandler::EventHandler(const NeoChatRoom *room, const RoomEvent *event)
    : m_room(room)
    , m_event(event)
{
}

QString EventHandler::getId() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getId called with m_event set to nullptr.";
        return {};
    }

    return !m_event->id().isEmpty() ? m_event->id() : m_event->transactionId();
}

MessageComponentType::Type EventHandler::messageComponentType() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "messageComponentType called with m_event set to nullptr.";
        return MessageComponentType::Other;
    }

    return MessageComponentType::typeForEvent(*m_event);
}

QVariantMap EventHandler::getAuthor(bool isPending) const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getAuthor called with m_room set to nullptr.";
        return {};
    }
    // If we have a room we can return an empty user by handing nullptr to m_room->getUser.
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getAuthor called with m_event set to nullptr. Returning empty user.";
        return m_room->getUser(QString());
    }

    const auto author = isPending ? m_room->localMember() : m_room->member(m_event->senderId());
    return m_room->getUser(author);
}

QString EventHandler::getAuthorDisplayName(bool isPending) const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getAuthorDisplayName called with m_room set to nullptr.";
        return {};
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getAuthorDisplayName called with m_event set to nullptr.";
        return {};
    }

    if (is<RoomMemberEvent>(*m_event) && !m_event->unsignedJson()[QStringLiteral("prev_content")][QStringLiteral("displayname")].isNull()
        && m_event->stateKey() == m_event->senderId()) {
        auto previousDisplayName = m_event->unsignedJson()[QStringLiteral("prev_content")][QStringLiteral("displayname")].toString().toHtmlEscaped();
        if (previousDisplayName.isEmpty()) {
            previousDisplayName = m_event->senderId();
        }
        return previousDisplayName;
    } else {
        const auto author = isPending ? m_room->localMember() : m_room->member(m_event->senderId());
        return author.htmlSafeDisplayName();
    }
}

QString EventHandler::singleLineAuthorDisplayname(bool isPending) const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getAuthorDisplayName called with m_room set to nullptr.";
        return {};
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getAuthorDisplayName called with m_event set to nullptr.";
        return {};
    }

    const auto author = isPending ? m_room->localMember() : m_room->member(m_event->senderId());
    auto displayName = author.displayName();
    displayName.replace(QStringLiteral("<br>\n"), QStringLiteral(" "));
    displayName.replace(QStringLiteral("<br>"), QStringLiteral(" "));
    displayName.replace(QStringLiteral("<br />\n"), QStringLiteral(" "));
    displayName.replace(QStringLiteral("<br />"), QStringLiteral(" "));
    displayName.replace(u'\n', QStringLiteral(" "));
    displayName.replace(u'\u2028', QStringLiteral(" "));
    return displayName;
}

QDateTime EventHandler::getTime(bool isPending, QDateTime lastUpdated) const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getTime called with m_event set to nullptr.";
        return {};
    }
    if (isPending && lastUpdated == QDateTime()) {
        qCWarning(EventHandling) << "a value must be provided for lastUpdated for a pending event.";
        return {};
    }

    return isPending ? lastUpdated : m_event->originTimestamp();
}

QString EventHandler::getTimeString(bool relative, QLocale::FormatType format, bool isPending, QDateTime lastUpdated) const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getTimeString called with m_event set to nullptr.";
        return {};
    }
    if (isPending && lastUpdated == QDateTime()) {
        qCWarning(EventHandling) << "a value must be provided for lastUpdated for a pending event.";
        return {};
    }

    auto ts = getTime(isPending, lastUpdated);
    if (ts.isValid()) {
        if (relative) {
            return m_format.formatRelativeDate(ts.toLocalTime().date(), format);
        } else {
            return QLocale().toString(ts.toLocalTime().time(), format);
        }
    }
    return {};
}

bool EventHandler::isHighlighted()
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "isHighlighted called with m_room set to nullptr.";
        return false;
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "isHighlighted called with m_event set to nullptr.";
        return false;
    }

    return !m_room->isDirectChat() && m_room->isEventHighlighted(m_event);
}

bool EventHandler::isHidden()
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "isHidden called with m_room set to nullptr.";
        return false;
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "isHidden called with m_event set to nullptr.";
        return false;
    }

    if (m_event->isStateEvent() && !NeoChatConfig::self()->showStateEvent()) {
        return true;
    }

    if (auto roomMemberEvent = eventCast<const RoomMemberEvent>(m_event)) {
        if ((roomMemberEvent->isJoin() || roomMemberEvent->isLeave()) && !NeoChatConfig::self()->showLeaveJoinEvent()) {
            return true;
        } else if (roomMemberEvent->isRename() && !roomMemberEvent->isJoin() && !roomMemberEvent->isLeave() && !NeoChatConfig::self()->showRename()) {
            return true;
        } else if (roomMemberEvent->isAvatarUpdate() && !roomMemberEvent->isJoin() && !roomMemberEvent->isLeave()
                   && !NeoChatConfig::self()->showAvatarUpdate()) {
            return true;
        }
    }

    if (m_event->isStateEvent() && eventCast<const StateEvent>(m_event)->repeatsState()) {
        return true;
    }

    // isReplacement?
    if (auto e = eventCast<const RoomMessageEvent>(m_event)) {
        if (!e->replacedEvent().isEmpty()) {
            return true;
        }
    }

    if (is<RedactionEvent>(*m_event) || is<ReactionEvent>(*m_event)) {
        return true;
    }

    if (auto e = eventCast<const RoomMessageEvent>(m_event)) {
        if (!e->replacedEvent().isEmpty() && e->replacedEvent() != e->id()) {
            return true;
        }
    }

    if (m_room->connection()->isIgnored(m_event->senderId())) {
        return true;
    }

    // hide ending live location beacons
    if (m_event->isStateEvent() && m_event->matrixType() == "org.matrix.msc3672.beacon_info"_ls && !m_event->contentJson()["live"_ls].toBool()) {
        return true;
    }

    return false;
}

Qt::TextFormat EventHandler::messageBodyInputFormat(const Quotient::RoomMessageEvent &event)
{
    if (event.mimeType().name() == "text/plain"_ls) {
        return Qt::PlainText;
    } else {
        return Qt::RichText;
    }
}

QString EventHandler::rawMessageBody(const Quotient::RoomMessageEvent &event)
{
    if (event.hasFileContent()) {
        auto fileCaption = event.content()->fileInfo()->originalName;
        if (fileCaption.isEmpty()) {
            fileCaption = event.plainBody();
        } else if (event.content()->fileInfo()->originalName != event.plainBody()) {
            fileCaption = event.plainBody() + " | "_ls + fileCaption;
        }
        return fileCaption;
    }

    QString body;
    if (event.hasTextContent() && event.content()) {
        body = static_cast<const EventContent::TextContent *>(event.content())->body;
    } else {
        body = event.plainBody();
    }
    return body;
}

QString EventHandler::getRichBody(bool stripNewlines) const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getRichBody called with m_event set to nullptr.";
        return {};
    }
    return getBody(m_event, Qt::RichText, stripNewlines);
}

QString EventHandler::getPlainBody(bool stripNewlines) const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getPlainBody called with m_event set to nullptr.";
        return {};
    }
    return getBody(m_event, Qt::PlainText, stripNewlines);
}

QString EventHandler::getMarkdownBody() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getMarkdownBody called with m_event set to nullptr.";
        return {};
    }

    if (!m_event->is<RoomMessageEvent>()) {
        qCWarning(EventHandling) << "getMarkdownBody called when m_event isn't a RoomMessageEvent.";
        return {};
    }

    const auto roomMessageEvent = eventCast<const RoomMessageEvent>(m_event);

    QString plainBody = roomMessageEvent->plainBody();
    plainBody.remove(TextRegex::removeReply);
    return plainBody;
}

QString EventHandler::getBody(const Quotient::RoomEvent *event, Qt::TextFormat format, bool stripNewlines) const
{
    if (event->isRedacted()) {
        auto reason = event->redactedBecause()->reason();
        return (reason.isEmpty()) ? i18n("<i>[This message was deleted]</i>") : i18n("<i>[This message was deleted: %1]</i>", reason);
    }

    const bool prettyPrint = (format == Qt::RichText);

    return switchOnType(
        *event,
        [this, format, stripNewlines](const RoomMessageEvent &event) {
            return getMessageBody(event, format, stripNewlines);
        },
        [](const StickerEvent &e) {
            return e.body();
        },
        [this, prettyPrint](const RoomMemberEvent &e) {
            // FIXME: Rewind to the name that was at the time of this event
            auto subjectName = m_room->member(e.userId()).htmlSafeDisplayName();
            if (e.membership() == Membership::Leave) {
                if (e.prevContent() && e.prevContent()->displayName) {
                    subjectName = sanitized(*e.prevContent()->displayName).toHtmlEscaped();
                }
            }

            if (prettyPrint) {
                subjectName = QStringLiteral("<a href=\"https://matrix.to/#/%1\">%2</a>").arg(e.userId(), subjectName);
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
                return (e.senderId() != e.userId())
                    ? i18n("has put %1 out of the room: %2", subjectName, e.contentJson()["reason"_ls].toString().toHtmlEscaped())
                    : i18n("left the room");
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
                QString reason(e.contentJson()["reason"_ls].toString().toHtmlEscaped());
                return reason.isEmpty() ? i18n("requested an invite") : i18n("requested an invite with reason: %1", reason);
            }
            default:;
            }
            return i18n("made something unknown");
        },
        [](const RoomCanonicalAliasEvent &e) {
            return (e.alias().isEmpty()) ? i18n("cleared the room main alias") : i18n("set the room main alias to: %1", e.alias());
        },
        [prettyPrint](const RoomNameEvent &e) {
            return (e.name().isEmpty()) ? i18n("cleared the room name") : i18n("set the room name to: %1", prettyPrint ? e.name().toHtmlEscaped() : e.name());
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
                ? i18n("upgraded the room to version %1", e.version().isEmpty() ? "1"_ls : (prettyPrint ? e.version().toHtmlEscaped() : e.version()))
                : i18n("created the room, version %1", e.version().isEmpty() ? "1"_ls : (prettyPrint ? e.version().toHtmlEscaped() : e.version()));
        },
        [](const RoomPowerLevelsEvent &) {
            return i18nc("'power level' means permission level", "changed the power levels for this room");
        },
        [](const LocationBeaconEvent &e) {
            return e.contentJson()["description"_ls].toString();
        },
        [](const ServerAclEvent &) {
            return i18n("changed the server access control lists for this room");
        },
        [](const WidgetEvent &e) {
            if (e.fullJson()["unsigned"_ls]["prev_content"_ls].toObject().isEmpty()) {
                return i18nc("[User] added <name> widget", "added %1 widget", e.contentJson()["name"_ls].toString());
            }
            if (e.contentJson().isEmpty()) {
                return i18nc("[User] removed <name> widget", "removed %1 widget", e.fullJson()["unsigned"_ls]["prev_content"_ls]["name"_ls].toString());
            }
            return i18nc("[User] configured <name> widget", "configured %1 widget", e.contentJson()["name"_ls].toString());
        },
        [prettyPrint](const StateEvent &e) {
            return e.stateKey().isEmpty() ? i18n("updated %1 state", e.matrixType())
                                          : i18n("updated %1 state for %2", e.matrixType(), prettyPrint ? e.stateKey().toHtmlEscaped() : e.stateKey());
        },
        [](const PollStartEvent &e) {
            return e.question();
        },
        i18n("Unknown event"));
}

QString EventHandler::getMessageBody(const RoomMessageEvent &event, Qt::TextFormat format, bool stripNewlines) const
{
    TextHandler textHandler;

    if (event.hasFileContent()) {
        auto fileCaption = event.content()->fileInfo()->originalName;
        if (fileCaption.isEmpty()) {
            fileCaption = event.plainBody();
        } else if (event.content()->fileInfo()->originalName != event.plainBody()) {
            fileCaption = event.plainBody() + " | "_ls + fileCaption;
        }
        textHandler.setData(fileCaption);
        return !fileCaption.isEmpty() ? textHandler.handleRecievePlainText(Qt::PlainText, stripNewlines) : i18n("a file");
    }

    QString body;
    if (event.hasTextContent() && event.content()) {
        body = static_cast<const EventContent::TextContent *>(event.content())->body;
    } else {
        body = event.plainBody();
    }

    textHandler.setData(body);

    Qt::TextFormat inputFormat;
    if (event.mimeType().name() == "text/plain"_ls) {
        inputFormat = Qt::PlainText;
    } else {
        inputFormat = Qt::RichText;
    }

    if (format == Qt::RichText) {
        return textHandler.handleRecieveRichText(inputFormat, m_room, &event, stripNewlines, event.isReplaced());
    } else {
        return textHandler.handleRecievePlainText(inputFormat, stripNewlines);
    }
}

QString EventHandler::getGenericBody() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getGenericBody called with m_event set to nullptr.";
        return {};
    }
    if (m_event->isRedacted()) {
        return i18n("<i>[This message was deleted]</i>");
    }

    return switchOnType(
        *m_event,
        [](const RoomMessageEvent &e) {
            Q_UNUSED(e)
            return i18n("sent a message");
        },
        [](const StickerEvent &e) {
            Q_UNUSED(e)
            return i18n("sent a sticker");
        },
        [](const RoomMemberEvent &e) {
            switch (e.membership()) {
            case Membership::Invite:
                if (e.repeatsState()) {
                    return i18n("reinvited someone to the room");
                }
                Q_FALLTHROUGH();
            case Membership::Join: {
                QString text{};
                // Part 1: invites and joins
                if (e.repeatsState()) {
                    text = i18n("joined the room (repeated)");
                } else if (e.changesMembership()) {
                    text = e.membership() == Membership::Invite ? i18n("invited someone to the room") : i18n("joined the room");
                }
                if (!text.isEmpty()) {
                    return text;
                }
                // Part 2: profile changes of joined members
                if (e.isRename()) {
                    if (!e.newDisplayName()) {
                        text = i18nc("their refers to a singular user", "cleared their display name");
                    } else {
                        text = i18nc("their refers to a singular user", "changed their display name");
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
                    return (e.senderId() != e.userId()) ? i18n("withdrew a user's invitation") : i18n("rejected the invitation");
                }

                if (e.prevContent() && e.prevContent()->membership == Membership::Ban) {
                    return (e.senderId() != e.userId()) ? i18n("unbanned a user") : i18n("self-unbanned");
                }
                return (e.senderId() != e.userId()) ? i18n("put a user out of the room") : i18n("left the room");
            case Membership::Ban:
                if (e.senderId() != e.userId()) {
                    return i18n("banned a user from the room");
                } else {
                    return i18n("self-banned from the room");
                }
            case Membership::Knock: {
                return i18n("requested an invite");
            }
            default:;
            }
            return i18n("made something unknown");
        },
        [](const RoomCanonicalAliasEvent &e) {
            return (e.alias().isEmpty()) ? i18n("cleared the room main alias") : i18n("set the room main alias");
        },
        [](const RoomNameEvent &e) {
            return (e.name().isEmpty()) ? i18n("cleared the room name") : i18n("set the room name");
        },
        [](const RoomTopicEvent &e) {
            return (e.topic().isEmpty()) ? i18n("cleared the topic") : i18n("set the topic");
        },
        [](const RoomAvatarEvent &) {
            return i18n("changed the room avatar");
        },
        [](const EncryptionEvent &) {
            return i18n("activated End-to-End Encryption");
        },
        [](const RoomCreateEvent &e) {
            return e.isUpgrade() ? i18n("upgraded the room version") : i18n("created the room");
        },
        [](const RoomPowerLevelsEvent &) {
            return i18nc("'power level' means permission level", "changed the power levels for this room");
        },
        [](const LocationBeaconEvent &) {
            return i18n("sent a live location beacon");
        },
        [](const ServerAclEvent &) {
            return i18n("changed the server access control lists for this room");
        },
        [](const WidgetEvent &e) {
            if (e.fullJson()["unsigned"_ls]["prev_content"_ls].toObject().isEmpty()) {
                return i18n("added a widget");
            }
            if (e.contentJson().isEmpty()) {
                return i18n("removed a widget");
            }
            return i18n("configured a widget");
        },
        [](const StateEvent &) {
            return i18n("updated the state");
        },
        [](const PollStartEvent &e) {
            Q_UNUSED(e);
            return i18n("started a poll");
        },
        i18n("Unknown event"));
}

QString EventHandler::subtitleText() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "subtitleText called with m_event set to nullptr.";
        return {};
    }
    return singleLineAuthorDisplayname() + (m_event->isStateEvent() ? QLatin1String(" ") : QLatin1String(": ")) + getPlainBody(true);
}

QVariantMap EventHandler::getMediaInfo() const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getMediaInfo called with m_room set to nullptr.";
        return {};
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getMediaInfo called with m_event set to nullptr.";
        return {};
    }
    return getMediaInfoForEvent(m_event);
}

QVariantMap EventHandler::getMediaInfoForEvent(const Quotient::RoomEvent *event) const
{
    QString eventId = event->id();

    // Get the file info for the event.
    const EventContent::FileInfo *fileInfo;
    bool isSticker = false;
    if (event->is<RoomMessageEvent>()) {
        auto roomMessageEvent = eventCast<const RoomMessageEvent>(event);
        if (!roomMessageEvent->hasFileContent()) {
            return {};
        }
        fileInfo = roomMessageEvent->content()->fileInfo();
    } else if (event->is<StickerEvent>()) {
        auto stickerEvent = eventCast<const StickerEvent>(event);
        fileInfo = &stickerEvent->image();
        isSticker = true;
    } else {
        return {};
    }

    return getMediaInfoFromFileInfo(fileInfo, eventId, false, isSticker);
}

QVariantMap EventHandler::getMediaInfoFromFileInfo(const EventContent::FileInfo *fileInfo, const QString &eventId, bool isThumbnail, bool isSticker) const
{
    QVariantMap mediaInfo;

    // Get the mxc URL for the media.
    if (!fileInfo->url().isValid() || fileInfo->url().scheme() != QStringLiteral("mxc") || eventId.isEmpty()) {
        mediaInfo["source"_ls] = QUrl();
    } else {
        QUrl source = m_room->makeMediaUrl(eventId, fileInfo->url());

        if (source.isValid()) {
            mediaInfo["source"_ls] = source;
        } else {
            mediaInfo["source"_ls] = QUrl();
        }
    }

    auto mimeType = fileInfo->mimeType;
    // Add the MIME type for the media if available.
    mediaInfo["mimeType"_ls] = mimeType.name();

    // Add the MIME type icon if available.
    mediaInfo["mimeIcon"_ls] = mimeType.iconName();

    // Add media size if available.
    mediaInfo["size"_ls] = fileInfo->payloadSize;

    mediaInfo["isSticker"_ls] = isSticker;

    // Add parameter depending on media type.
    if (mimeType.name().contains(QStringLiteral("image"))) {
        if (auto castInfo = static_cast<const EventContent::ImageContent *>(fileInfo)) {
            mediaInfo["width"_ls] = castInfo->imageSize.width();
            mediaInfo["height"_ls] = castInfo->imageSize.height();

            // TODO: Images in certain formats (e.g. WebP) will be erroneously marked as animated, even if they are static.
            mediaInfo["animated"_ls] = QMovie::supportedFormats().contains(mimeType.preferredSuffix().toUtf8());

            if (!isThumbnail) {
                QVariantMap tempInfo;
                auto thumbnailInfo = getMediaInfoFromFileInfo(castInfo->thumbnailInfo(), eventId, true);
                if (thumbnailInfo["source"_ls].toUrl().scheme() == "mxc"_ls) {
                    tempInfo = thumbnailInfo;
                } else {
                    QString blurhash = castInfo->originalInfoJson["xyz.amorgan.blurhash"_ls].toString();
                    if (blurhash.isEmpty()) {
                        tempInfo["source"_ls] = QUrl();
                    } else {
                        tempInfo["source"_ls] = QUrl("image://blurhash/"_ls + blurhash);
                    }
                }
                mediaInfo["tempInfo"_ls] = tempInfo;
            }
        }
    }
    if (mimeType.name().contains(QStringLiteral("video"))) {
        if (auto castInfo = static_cast<const EventContent::VideoContent *>(fileInfo)) {
            mediaInfo["width"_ls] = castInfo->imageSize.width();
            mediaInfo["height"_ls] = castInfo->imageSize.height();
            mediaInfo["duration"_ls] = castInfo->duration;

            if (!isThumbnail) {
                QVariantMap tempInfo;
                auto thumbnailInfo = getMediaInfoFromFileInfo(castInfo->thumbnailInfo(), eventId, true);
                if (thumbnailInfo["source"_ls].toUrl().scheme() == "mxc"_ls) {
                    tempInfo = thumbnailInfo;
                } else {
                    QString blurhash = castInfo->originalInfoJson["xyz.amorgan.blurhash"_ls].toString();
                    if (blurhash.isEmpty()) {
                        tempInfo["source"_ls] = QUrl();
                    } else {
                        tempInfo["source"_ls] = QUrl("image://blurhash/"_ls + blurhash);
                    }
                }
                mediaInfo["tempInfo"_ls] = tempInfo;
            }
        }
    }
    if (mimeType.name().contains(QStringLiteral("audio"))) {
        if (auto castInfo = static_cast<const EventContent::AudioContent *>(fileInfo)) {
            mediaInfo["duration"_ls] = castInfo->duration;
        }
    }

    return mediaInfo;
}

bool EventHandler::hasReply() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "hasReply called with m_event set to nullptr.";
        return false;
    }
    return !m_event->contentJson()["m.relates_to"_ls].toObject()["m.in_reply_to"_ls].toObject()["event_id"_ls].toString().isEmpty();
}

QString EventHandler::getReplyId() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getReplyId called with m_event set to nullptr.";
        return {};
    }
    return m_event->contentJson()["m.relates_to"_ls].toObject()["m.in_reply_to"_ls].toObject()["event_id"_ls].toString();
}

MessageComponentType::Type EventHandler::replyMessageComponentType() const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "replyMessageComponentType called with m_room set to nullptr.";
        return MessageComponentType::Other;
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "replyMessageComponentType called with m_event set to nullptr.";
        return MessageComponentType::Other;
    }

    auto replyEvent = m_room->getReplyForEvent(*m_event);
    if (replyEvent == nullptr) {
        return MessageComponentType::Other;
    }
    return MessageComponentType::typeForEvent(*replyEvent);
}

QVariantMap EventHandler::getReplyAuthor() const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getReplyAuthor called with m_room set to nullptr.";
        return {};
    }
    // If we have a room we can return an empty user by handing nullptr to m_room->getUser.
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getReplyAuthor called with m_event set to nullptr. Returning empty user.";
        return m_room->getUser(QString());
    }

    auto replyPtr = m_room->getReplyForEvent(*m_event);

    if (replyPtr) {
        return m_room->getUser(replyPtr->senderId());
    } else {
        return m_room->getUser(QString());
    }
}

QString EventHandler::getReplyRichBody(bool stripNewlines) const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getReplyRichBody called with m_room set to nullptr.";
        return {};
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getReplyRichBody called with m_event set to nullptr.";
        return {};
    }

    auto replyEvent = m_room->getReplyForEvent(*m_event);
    if (replyEvent == nullptr) {
        return {};
    }

    return getBody(replyEvent, Qt::RichText, stripNewlines);
}

QString EventHandler::getReplyPlainBody(bool stripNewlines) const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getReplyPlainBody called with m_room set to nullptr.";
        return {};
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getReplyPlainBody called with m_event set to nullptr.";
        return {};
    }

    auto replyEvent = m_room->getReplyForEvent(*m_event);
    if (replyEvent == nullptr) {
        return {};
    }

    return getBody(replyEvent, Qt::PlainText, stripNewlines);
}

QVariantMap EventHandler::getReplyMediaInfo() const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getReplyMediaInfo called with m_room set to nullptr.";
        return {};
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getReplyMediaInfo called with m_event set to nullptr.";
        return {};
    }

    auto replyPtr = m_room->getReplyForEvent(*m_event);
    if (!replyPtr) {
        return {};
    }
    return getMediaInfoForEvent(replyPtr);
}

bool EventHandler::isThreaded() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "isThreaded called with m_event set to nullptr.";
        return false;
    }

    return (m_event->contentPart<QJsonObject>("m.relates_to"_ls).contains("rel_type"_ls)
            && m_event->contentPart<QJsonObject>("m.relates_to"_ls)["rel_type"_ls].toString() == "m.thread"_ls)
        || (!m_event->unsignedPart<QJsonObject>("m.relations"_ls).isEmpty() && m_event->unsignedPart<QJsonObject>("m.relations"_ls).contains("m.thread"_ls));
}

QString EventHandler::threadRoot() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "threadRoot called with m_event set to nullptr.";
        return {};
    }

    // Get the thread root ID from m.relates_to if it exists.
    if (m_event->contentPart<QJsonObject>("m.relates_to"_ls).contains("rel_type"_ls)
        && m_event->contentPart<QJsonObject>("m.relates_to"_ls)["rel_type"_ls].toString() == "m.thread"_ls) {
        return m_event->contentPart<QJsonObject>("m.relates_to"_ls)["event_id"_ls].toString();
    }
    // For thread root events they have an m.relations in the unsigned part with a m.thread object.
    // If so return the event ID as it is the root.
    if (!m_event->unsignedPart<QJsonObject>("m.relations"_ls).isEmpty() && m_event->unsignedPart<QJsonObject>("m.relations"_ls).contains("m.thread"_ls)) {
        return getId();
    }
    return {};
}

float EventHandler::getLatitude() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getLatitude called with m_event set to nullptr.";
        return -100.0;
    }

    const auto geoUri = m_event->contentJson()["geo_uri"_ls].toString();
    if (geoUri.isEmpty()) {
        return -100.0; // latitude runs from -90deg to +90deg so -100 is out of range.
    }
    const auto latitude = geoUri.split(u';')[0].split(u':')[1].split(u',')[0];
    return latitude.toFloat();
}

float EventHandler::getLongitude() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getLongitude called with m_event set to nullptr.";
        return -200.0;
    }

    const auto geoUri = m_event->contentJson()["geo_uri"_ls].toString();
    if (geoUri.isEmpty()) {
        return -200.0; // longitude runs from -180deg to +180deg so -200 is out of range.
    }
    const auto latitude = geoUri.split(u';')[0].split(u':')[1].split(u',')[1];
    return latitude.toFloat();
}

QString EventHandler::getLocationAssetType() const
{
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getLocationAssetType called with m_event set to nullptr.";
        return {};
    }

    const auto assetType = m_event->contentJson()["org.matrix.msc3488.asset"_ls].toObject()["type"_ls].toString();
    if (assetType.isEmpty()) {
        return {};
    }
    return assetType;
}

bool EventHandler::hasReadMarkers() const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "hasReadMarkers called with m_room set to nullptr.";
        return false;
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "hasReadMarkers called with m_event set to nullptr.";
        return false;
    }

    auto userIds = m_room->userIdsAtEvent(m_event->id());
    userIds.remove(m_room->localMember().id());
    return userIds.size() > 0;
}

QVariantList EventHandler::getReadMarkers(int maxMarkers) const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getReadMarkers called with m_room set to nullptr.";
        return {};
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getReadMarkers called with m_event set to nullptr.";
        return {};
    }

    auto userIds_temp = m_room->userIdsAtEvent(m_event->id());
    userIds_temp.remove(m_room->localMember().id());

    auto userIds = userIds_temp.values();
    if (userIds.count() > maxMarkers) {
        userIds = userIds.mid(0, maxMarkers);
    }

    QVariantList users;
    users.reserve(userIds.size());
    for (const auto &userId : userIds) {
        auto user = m_room->member(userId);
        users += m_room->getUser(user);
    }

    return users;
}

QString EventHandler::getNumberExcessReadMarkers(int maxMarkers) const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getNumberExcessReadMarkers called with m_room set to nullptr.";
        return {};
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getNumberExcessReadMarkers called with m_event set to nullptr.";
        return {};
    }

    auto userIds = m_room->userIdsAtEvent(m_event->id());
    userIds.remove(m_room->localMember().id());

    if (userIds.count() > maxMarkers) {
        return QStringLiteral("+ ") + QString::number(userIds.count() - maxMarkers);
    } else {
        return QString();
    }
}

QString EventHandler::getReadMarkersString() const
{
    if (m_room == nullptr) {
        qCWarning(EventHandling) << "getReadMarkersString called with m_room set to nullptr.";
        return {};
    }
    if (m_event == nullptr) {
        qCWarning(EventHandling) << "getReadMarkersString called with m_event set to nullptr.";
        return {};
    }

    auto userIds = m_room->userIdsAtEvent(m_event->id());
    userIds.remove(m_room->localMember().id());

    /**
     * The string ends up in the form
     * "x users: user1DisplayName, user2DisplayName, etc."
     */
    QString readMarkersString = i18np("1 user: ", "%1 users: ", userIds.size());
    for (const auto &userId : userIds) {
        auto user = m_room->member(userId);
        auto displayName = user.displayName();
        if (displayName.isEmpty()) {
            displayName = userId;
        }
        readMarkersString += displayName + i18nc("list separator", ", ");
    }
    readMarkersString.chop(2);
    return readMarkersString;
}

#include "moc_eventhandler.cpp"
