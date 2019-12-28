#include "spectralroom.h"

#include <cmark.h>

#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMetaObject>
#include <QMimeDatabase>
#include <QTextDocument>
#include <functional>

#include "connection.h"
#include "csapi/account-data.h"
#include "csapi/content-repo.h"
#include "csapi/leaving.h"
#include "csapi/room_state.h"
#include "csapi/rooms.h"
#include "csapi/typing.h"
#include "events/accountdataevents.h"
#include "events/reactionevent.h"
#include "events/roommessageevent.h"
#include "events/typingevent.h"
#include "jobs/downloadfilejob.h"
#include "user.h"
#include "utils.h"

SpectralRoom::SpectralRoom(Connection* connection,
                           QString roomId,
                           JoinState joinState)
    : Room(connection, std::move(roomId), joinState) {
  connect(this, &SpectralRoom::notificationCountChanged, this,
          &SpectralRoom::countChanged);
  connect(this, &SpectralRoom::highlightCountChanged, this,
          &SpectralRoom::countChanged);
  connect(this, &Room::fileTransferCompleted, this, [=] {
    setFileUploadingProgress(0);
    setHasFileUploading(false);
  });
}

void SpectralRoom::uploadFile(const QUrl& url, const QString& body) {
  if (url.isEmpty())
    return;

  QString txnId = postFile(body.isEmpty() ? url.fileName() : body, url, false);
  setHasFileUploading(true);
  connect(this, &Room::fileTransferCompleted,
          [=](QString id, QUrl /*localFile*/, QUrl /*mxcUrl*/) {
            if (id == txnId) {
              setFileUploadingProgress(0);
              setHasFileUploading(false);
            }
          });
  connect(this, &Room::fileTransferFailed, [=](QString id, QString /*error*/) {
    if (id == txnId) {
      setFileUploadingProgress(0);
      setHasFileUploading(false);
    }
  });
  connect(
      this, &Room::fileTransferProgress,
      [=](QString id, qint64 progress, qint64 total) {
        if (id == txnId) {
          qDebug() << "Progress:" << progress << total;
          setFileUploadingProgress(int(float(progress) / float(total) * 100));
        }
      });
}

void SpectralRoom::acceptInvitation() {
  connection()->joinRoom(id());
}

void SpectralRoom::forget() {
  connection()->forgetRoom(id());
}

QVariantList SpectralRoom::getUsersTyping() const {
  auto users = usersTyping();
  users.removeAll(localUser());
  QVariantList userVariants;
  for (User* user : users) {
    userVariants.append(QVariant::fromValue(user));
  }
  return userVariants;
}

void SpectralRoom::sendTypingNotification(bool isTyping) {
  connection()->callApi<SetTypingJob>(BackgroundRequest, localUser()->id(),
                                      id(), isTyping, 10000);
}

QString SpectralRoom::lastEvent() const {
  for (auto i = messageEvents().rbegin(); i < messageEvents().rend(); i++) {
    const RoomEvent* evt = i->get();

    if (is<RedactionEvent>(*evt) || is<ReactionEvent>(*evt))
      continue;
    if (evt->isRedacted())
      continue;

    if (evt->isStateEvent() &&
        static_cast<const StateEventBase&>(*evt).repeatsState())
      continue;

    if (auto e = eventCast<const RoomMessageEvent>(evt)) {
      if (!e->replacedEvent().isEmpty() && e->replacedEvent() != e->id()) {
        continue;
      }
    }

    if (connection()->isIgnored(user(evt->senderId())))
      continue;

    return user(evt->senderId())->displayname() +
           (evt->isStateEvent() ? " " : ": ") + eventToString(*evt);
  }
  return "";
}

bool SpectralRoom::isEventHighlighted(const RoomEvent* e) const {
  return highlights.contains(e);
}

void SpectralRoom::checkForHighlights(const Quotient::TimelineItem& ti) {
  auto localUserId = localUser()->id();
  if (ti->senderId() == localUserId)
    return;
  if (auto* e = ti.viewAs<RoomMessageEvent>()) {
    const auto& text = e->plainBody();
    if (text.contains(localUserId) ||
        text.contains(roomMembername(localUserId)))
      highlights.insert(e);
  }
}

void SpectralRoom::onAddNewTimelineEvents(timeline_iter_t from) {
  std::for_each(from, messageEvents().cend(),
                [this](const TimelineItem& ti) { checkForHighlights(ti); });
}

void SpectralRoom::onAddHistoricalTimelineEvents(rev_iter_t from) {
  std::for_each(from, messageEvents().crend(),
                [this](const TimelineItem& ti) { checkForHighlights(ti); });
}

void SpectralRoom::onRedaction(const RoomEvent& prevEvent,
                               const RoomEvent& /*after*/) {
  if (const auto& e = eventCast<const ReactionEvent>(&prevEvent)) {
    if (auto relatedEventId = e->relation().eventId;
        !relatedEventId.isEmpty()) {
      emit updatedEvent(relatedEventId);
    }
  }
}

void SpectralRoom::countChanged() {
  if (displayed() && !hasUnreadMessages()) {
    resetNotificationCount();
    resetHighlightCount();
  }
}

QDateTime SpectralRoom::lastActiveTime() const {
  if (timelineSize() == 0)
    return QDateTime();
  return messageEvents().rbegin()->get()->timestamp();
}

int SpectralRoom::savedTopVisibleIndex() const {
  return firstDisplayedMarker() == timelineEdge()
             ? 0
             : int(firstDisplayedMarker() - messageEvents().rbegin());
}

int SpectralRoom::savedBottomVisibleIndex() const {
  return lastDisplayedMarker() == timelineEdge()
             ? 0
             : int(lastDisplayedMarker() - messageEvents().rbegin());
}

void SpectralRoom::saveViewport(int topIndex, int bottomIndex) {
  if (topIndex == -1 || bottomIndex == -1 ||
      (bottomIndex == savedBottomVisibleIndex() &&
       (bottomIndex == 0 || topIndex == savedTopVisibleIndex())))
    return;
  if (bottomIndex == 0) {
    setFirstDisplayedEventId({});
    setLastDisplayedEventId({});
    return;
  }
  setFirstDisplayedEvent(maxTimelineIndex() - topIndex);
  setLastDisplayedEvent(maxTimelineIndex() - bottomIndex);
}

QVariantList SpectralRoom::getUsers(const QString& keyword) const {
  const auto userList = users();
  QVariantList matchedList;
  for (const auto u : userList)
    if (u->displayname(this).contains(keyword, Qt::CaseInsensitive)) {
      matchedList.append(QVariant::fromValue(u));
    }

  return matchedList;
}

QUrl SpectralRoom::urlToMxcUrl(QUrl mxcUrl) {
  return DownloadFileJob::makeRequestUrl(connection()->homeserver(), mxcUrl);
}

QString SpectralRoom::avatarMediaId() const {
  if (const auto avatar = Room::avatarMediaId(); !avatar.isEmpty()) {
    return avatar;
  }

  // Use the first (excluding self) user's avatar for direct chats
  const auto dcUsers = directChatUsers();
  for (const auto u : dcUsers) {
    if (u != localUser()) {
      return u->avatarMediaId();
    }
  }

  return {};
}

QString SpectralRoom::eventToString(const RoomEvent& evt,
                                    Qt::TextFormat format,
                                    bool removeReply) const {
  const bool prettyPrint = (format == Qt::RichText);

  using namespace Quotient;
  return visit(
      evt,
      [prettyPrint, removeReply](const RoomMessageEvent& e) {
        using namespace MessageEventContent;

        if (prettyPrint && e.hasTextContent() &&
            e.mimeType().name() != "text/plain") {
          auto htmlBody = static_cast<const TextContent*>(e.content())->body;
          if (removeReply) {
            htmlBody.remove(utils::removeRichReplyRegex);
          }
          htmlBody.replace(utils::userPillRegExp, "<b>\\1</b>");
          htmlBody.replace(utils::strikethroughRegExp, "<s>\\1</s>");
          htmlBody.push_front("<style>pre {white-space: pre-wrap}</style>");
          return htmlBody;
        }

        if (e.hasFileContent()) {
          auto fileCaption =
              e.content()->fileInfo()->originalName.toHtmlEscaped();
          if (fileCaption.isEmpty()) {
            fileCaption = prettyPrint ? Quotient::prettyPrint(e.plainBody())
                                      : e.plainBody();
          } else if (e.content()->fileInfo()->originalName != e.plainBody()) {
            fileCaption = e.plainBody() + " | " + fileCaption;
          }
          return !fileCaption.isEmpty() ? fileCaption : tr("a file");
        }

        QString plainBody;
        if (e.hasTextContent() && e.content()) {
            plainBody = static_cast<const TextContent*>(e.content())->body;
        } else {
            plainBody = e.plainBody();
        }

        if (prettyPrint) {
          if (removeReply) {
            plainBody.remove(utils::removeReplyRegex);
          }
          return Quotient::prettyPrint(plainBody);
        }
        if (removeReply) {
          return plainBody.remove(utils::removeReplyRegex);
        }
        return plainBody;
      },
      [this](const RoomMemberEvent& e) {
        // FIXME: Rewind to the name that was at the time of this event
        auto subjectName = this->user(e.userId())->displayname();
        // The below code assumes senderName output in AuthorRole
        switch (e.membership()) {
          case MembershipType::Invite:
            if (e.repeatsState())
              return tr("reinvited %1 to the room").arg(subjectName);
          case MembershipType::Join: {
            if (e.repeatsState())
              return tr("joined the room (repeated)");
            if (!e.prevContent() ||
                e.membership() != e.prevContent()->membership) {
              return e.membership() == MembershipType::Invite
                         ? tr("invited %1 to the room").arg(subjectName)
                         : tr("joined the room");
            }
            QString text{};
            if (e.isRename()) {
              if (e.displayName().isEmpty())
                text = tr("cleared their display name");
              else
                text = tr("changed their display name to %1")
                           .arg(e.displayName().toHtmlEscaped());
            }
            if (e.isAvatarUpdate()) {
              if (!text.isEmpty())
                text += " and ";
              if (e.avatarUrl().isEmpty())
                text += tr("cleared their avatar");
              else if (e.prevContent()->avatarUrl.isEmpty())
                text += tr("set an avatar");
              else
                text += tr("updated their avatar");
            }
            return text;
          }
          case MembershipType::Leave:
            if (e.prevContent() &&
                e.prevContent()->membership == MembershipType::Invite) {
              return (e.senderId() != e.userId())
                         ? tr("withdrew %1's invitation").arg(subjectName)
                         : tr("rejected the invitation");
            }

            if (e.prevContent() &&
                e.prevContent()->membership == MembershipType::Ban) {
              return (e.senderId() != e.userId())
                         ? tr("unbanned %1").arg(subjectName)
                         : tr("self-unbanned");
            }
            return (e.senderId() != e.userId())
                       ? tr("has put %1 out of the room: %2")
                             .arg(subjectName, e.contentJson()["reason"_ls]
                                                   .toString()
                                                   .toHtmlEscaped())
                       : tr("left the room");
          case MembershipType::Ban:
            return (e.senderId() != e.userId())
                       ? tr("banned %1 from the room: %2")
                             .arg(subjectName, e.contentJson()["reason"_ls]
                                                   .toString()
                                                   .toHtmlEscaped())
                       : tr("self-banned from the room");
          case MembershipType::Knock:
            return tr("knocked");
          default:;
        }
        return tr("made something unknown");
      },
      [](const RoomAliasesEvent& e) {
        return tr("has set room aliases on server %1 to: %2")
            .arg(e.stateKey(), QLocale().createSeparatedList(e.aliases()));
      },
      [](const RoomCanonicalAliasEvent& e) {
        return (e.alias().isEmpty())
                   ? tr("cleared the room main alias")
                   : tr("set the room main alias to: %1").arg(e.alias());
      },
      [](const RoomNameEvent& e) {
        return (e.name().isEmpty()) ? tr("cleared the room name")
                                    : tr("set the room name to: %1")
                                          .arg(e.name().toHtmlEscaped());
      },
      [prettyPrint](const RoomTopicEvent& e) {
        return (e.topic().isEmpty())
                   ? tr("cleared the topic")
                   : tr("set the topic to: %1")
                         .arg(prettyPrint ? Quotient::prettyPrint(e.topic())
                                          : e.topic());
      },
      [](const RoomAvatarEvent&) { return tr("changed the room avatar"); },
      [](const EncryptionEvent&) {
        return tr("activated End-to-End Encryption");
      },
      [](const RoomCreateEvent& e) {
        return (e.isUpgrade() ? tr("upgraded the room to version %1")
                              : tr("created the room, version %1"))
            .arg(e.version().isEmpty() ? "1" : e.version().toHtmlEscaped());
      },
      [](const StateEventBase& e) {
        // A small hack for state events from TWIM bot
        return e.stateKey() == "twim"
                   ? tr("updated the database", "TWIM bot updated the database")
                   : e.stateKey().isEmpty()
                         ? tr("updated %1 state", "%1 - Matrix event type")
                               .arg(e.matrixType())
                         : tr("updated %1 state for %2",
                              "%1 - Matrix event type, %2 - state key")
                               .arg(e.matrixType(),
                                    e.stateKey().toHtmlEscaped());
      },
      tr("Unknown event"));
}

void SpectralRoom::changeAvatar(QUrl localFile) {
  const auto job = connection()->uploadFile(localFile.toLocalFile());
  if (isJobRunning(job)) {
    connect(job, &BaseJob::success, this, [this, job] {
      connection()->callApi<SetRoomStateJob>(
          id(), "m.room.avatar", QJsonObject{{"url", job->contentUri()}});
    });
  }
}

void SpectralRoom::addLocalAlias(const QString& alias) {
  auto aliases = localAliases();
  if (aliases.contains(alias))
    return;

  aliases += alias;

  setLocalAliases(aliases);
}

void SpectralRoom::removeLocalAlias(const QString& alias) {
  auto aliases = localAliases();
  if (!aliases.contains(alias))
    return;

  aliases.removeAll(alias);

  setLocalAliases(aliases);
}

QString SpectralRoom::markdownToHTML(const QString& markdown) {
  const auto str = markdown.toUtf8();
  char* tmp_buf =
      cmark_markdown_to_html(str.constData(), str.size(), CMARK_OPT_DEFAULT);

  const std::string html(tmp_buf);

  free(tmp_buf);

  auto result = QString::fromStdString(html).trimmed();

  result.replace("<p>", "");
  result.replace("</p>", "");

  return result;
}

void SpectralRoom::postArbitaryMessage(const QString& text,
                                       MessageEventType type,
                                       const QString& replyEventId) {
  const auto parsedHTML = markdownToHTML(text);
  const bool isRichText = Qt::mightBeRichText(parsedHTML);

  if (isRichText) {  // Markdown
    postHtmlMessage(text, parsedHTML, type, replyEventId);
  } else {  // Plain text
    postPlainMessage(text, type, replyEventId);
  }
}

QString msgTypeToString(MessageEventType msgType) {
  switch (msgType) {
    case MessageEventType::Text:
      return "m.text";
    case MessageEventType::File:
      return "m.file";
    case MessageEventType::Audio:
      return "m.audio";
    case MessageEventType::Emote:
      return "m.emote";
    case MessageEventType::Image:
      return "m.image";
    case MessageEventType::Video:
      return "m.video";
    case MessageEventType::Notice:
      return "m.notice";
    case MessageEventType::Location:
      return "m.location";
    default:
      return "m.text";
  }
}

void SpectralRoom::postPlainMessage(const QString& text,
                                    MessageEventType type,
                                    const QString& replyEventId) {
  bool isReply = !replyEventId.isEmpty();
  const auto replyIt = findInTimeline(replyEventId);
  if (replyIt == timelineEdge())
    isReply = false;

  if (isReply) {
    const auto& replyEvt = **replyIt;

    QJsonObject json{{"msgtype", msgTypeToString(type)},
                     {"body", "> <" + replyEvt.senderId() + "> " +
                                  eventToString(replyEvt) + "\n\n" + text},
                     {"format", "org.matrix.custom.html"},
                     {"m.relates_to",
                      QJsonObject{{"m.in_reply_to",
                                   QJsonObject{{"event_id", replyEventId}}}}},
                     {"formatted_body",
                      "<mx-reply><blockquote><a href=\"https://matrix.to/#/" +
                          id() + "/" + replyEventId +
                          "\">In reply to</a> <a href=\"https://matrix.to/#/" +
                          replyEvt.senderId() + "\">" + replyEvt.senderId() +
                          "</a><br>" + eventToString(replyEvt, Qt::RichText) +
                          "</blockquote></mx-reply>" + text.toHtmlEscaped()}};
    postJson("m.room.message", json);

    return;
  }

  Room::postMessage(text, type);
}

void SpectralRoom::postHtmlMessage(const QString& text,
                                   const QString& html,
                                   MessageEventType type,
                                   const QString& replyEventId) {
  bool isReply = !replyEventId.isEmpty();
  const auto replyIt = findInTimeline(replyEventId);
  if (replyIt == timelineEdge())
    isReply = false;

  if (isReply) {
    const auto& replyEvt = **replyIt;

    QJsonObject json{{"msgtype", msgTypeToString(type)},
                     {"body", "> <" + replyEvt.senderId() + "> " +
                                  eventToString(replyEvt) + "\n\n" + text},
                     {"format", "org.matrix.custom.html"},
                     {"m.relates_to",
                      QJsonObject{{"m.in_reply_to",
                                   QJsonObject{{"event_id", replyEventId}}}}},
                     {"formatted_body",
                      "<mx-reply><blockquote><a href=\"https://matrix.to/#/" +
                          id() + "/" + replyEventId +
                          "\">In reply to</a> <a href=\"https://matrix.to/#/" +
                          replyEvt.senderId() + "\">" + replyEvt.senderId() +
                          "</a><br>" + eventToString(replyEvt, Qt::RichText) +
                          "</blockquote></mx-reply>" + html}};
    postJson("m.room.message", json);

    return;
  }

  Room::postHtmlMessage(text, html, type);
}

void SpectralRoom::toggleReaction(const QString& eventId,
                                  const QString& reaction) {
  if (eventId.isEmpty() || reaction.isEmpty())
    return;

  const auto eventIt = findInTimeline(eventId);
  if (eventIt == timelineEdge())
    return;

  const auto& evt = **eventIt;

  QStringList redactEventIds;  // What if there are multiple reaction events?

  const auto& annotations = relatedEvents(evt, EventRelation::Annotation());
  if (!annotations.isEmpty()) {
    for (const auto& a : annotations) {
      if (auto e = eventCast<const ReactionEvent>(a)) {
        if (e->relation().key != reaction)
          continue;

        if (e->senderId() == localUser()->id()) {
          redactEventIds.push_back(e->id());
          break;
        }
      }
    }
  }

  if (!redactEventIds.isEmpty()) {
    for (const auto& redactEventId : redactEventIds) {
      redactEvent(redactEventId);
    }
  } else {
    postReaction(eventId, reaction);
  }
}

bool SpectralRoom::containsUser(QString userID) const {
  auto u = Room::user(userID);

  if (!u)
    return false;

  return Room::memberJoinState(u) != JoinState::Leave;
}
