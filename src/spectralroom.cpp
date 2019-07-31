#include "spectralroom.h"

#include "connection.h"
#include "user.h"

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

#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMetaObject>
#include <QMimeDatabase>
#include <QTextDocument>

#include <cmark.h>

#include "utils.h"

SpectralRoom::SpectralRoom(Connection* connection,
                           QString roomId,
                           JoinState joinState)
    : Room(connection, std::move(roomId), joinState) {
  connect(this, &SpectralRoom::notificationCountChanged, this,
          &SpectralRoom::countChanged);
  connect(this, &SpectralRoom::highlightCountChanged, this,
          &SpectralRoom::countChanged);
  connect(this, &Room::addedMessages, this, [=] { setBusy(false); });
  connect(this, &Room::fileTransferCompleted, this, [=] {
    setFileUploadingProgress(0);
    setHasFileUploading(false);
  });
  connect(this, &Room::accountDataChanged, this, [=](QString type) {
    if (type == backgroundEventType)
      emit backgroundChanged();
  });
}

inline QString getMIME(const QUrl& fileUrl) {
  return QMimeDatabase().mimeTypeForFile(fileUrl.toLocalFile()).name();
}

inline QSize getImageSize(const QUrl& imageUrl) {
  QImageReader reader(imageUrl.toLocalFile());
  return reader.size();
}

void SpectralRoom::uploadFile(const QUrl& url, const QString& body) {
  if (url.isEmpty())
    return;

  QString txnId = postFile(body.isEmpty() ? url.fileName() : body, url, false);
  setHasFileUploading(true);
  connect(this, &Room::fileTransferCompleted,
          [=](QString id, QUrl localFile, QUrl mxcUrl) {
            if (id == txnId) {
              setFileUploadingProgress(0);
              setHasFileUploading(false);
            }
          });
  connect(this, &Room::fileTransferFailed, [=](QString id, QString error) {
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

bool SpectralRoom::hasUsersTyping() {
  QList<User*> users = usersTyping();
  if (users.isEmpty())
    return false;
  int count = users.length();
  if (users.contains(localUser()))
    count--;
  return count != 0;
}

QVariantList SpectralRoom::getUsersTyping() {
  QList<User*> users = usersTyping();
  users.removeOne(localUser());
  QVariantList out;
  for (User* user : users) {
    out.append(QVariant::fromValue(user));
  }
  return out;
}

void SpectralRoom::sendTypingNotification(bool isTyping) {
  connection()->callApi<SetTypingJob>(BackgroundRequest, localUser()->id(),
                                      id(), isTyping, 10000);
}

QString SpectralRoom::lastEvent() {
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
      if (!e->replacedEvent().isEmpty()) {
        continue;
      }
    }

    if (connection()->isIgnored(user(evt->senderId())))
      continue;

    return user(evt->senderId())->displayname() +
           (evt->isStateEvent() ? " " : ": ") +
           utils::removeReply(eventToString(*evt));
  }
  return "";
}

bool SpectralRoom::isEventHighlighted(const RoomEvent* e) const {
  return highlights.contains(e);
}

void SpectralRoom::checkForHighlights(const QMatrixClient::TimelineItem& ti) {
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

void SpectralRoom::countChanged() {
  if (displayed() && !hasUnreadMessages()) {
    resetNotificationCount();
    resetHighlightCount();
  }
}

QDateTime SpectralRoom::lastActiveTime() {
  if (timelineSize() == 0)
    return QDateTime();
  return messageEvents().rbegin()->get()->timestamp();
}

int SpectralRoom::savedTopVisibleIndex() const {
  return firstDisplayedMarker() == timelineEdge()
             ? 0
             : firstDisplayedMarker() - messageEvents().rbegin();
}

int SpectralRoom::savedBottomVisibleIndex() const {
  return lastDisplayedMarker() == timelineEdge()
             ? 0
             : lastDisplayedMarker() - messageEvents().rbegin();
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

void SpectralRoom::getPreviousContent(int limit) {
  setBusy(true);
  Room::getPreviousContent(limit);
}

QVariantList SpectralRoom::getUsers(const QString& prefix) {
  auto userList = users();
  QVariantList matchedList;
  for (auto u : userList)
    if (u->displayname(this).toLower().startsWith(prefix.toLower()))
      matchedList.append(QVariant::fromValue(u));

  return matchedList;
}

QUrl SpectralRoom::urlToMxcUrl(QUrl mxcUrl) {
  return DownloadFileJob::makeRequestUrl(connection()->homeserver(), mxcUrl);
}

QUrl SpectralRoom::backgroundUrl() {
  return hasAccountData(backgroundEventType)
             ? QUrl(accountData(backgroundEventType)
                        .get()
                        ->contentJson()["url"]
                        .toString())
             : QUrl();
}

void SpectralRoom::setBackgroundUrl(QUrl url) {
  if (url.isEmpty() || url == backgroundUrl())
    return;

  connection()->callApi<SetAccountDataPerRoomJob>(
      localUser()->id(), id(), backgroundEventType,
      QJsonObject{{"url", url.toString()}});
}

void SpectralRoom::setBackgroundFromLocalFile(QUrl url) {
  if (url.isEmpty())
    return;

  auto txnId = connection()->generateTxnId();
  Room::uploadFile(txnId, url);

  connect(this, &Room::fileTransferCompleted,
          [=](QString id, QUrl localFile, QUrl mxcUrl) {
            if (id == txnId) {
              setBackgroundUrl(mxcUrl);
            }
          });
}

void SpectralRoom::clearBackground() {
  connection()->callApi<SetAccountDataPerRoomJob>(
      localUser()->id(), id(), backgroundEventType, QJsonObject{});
}

QString SpectralRoom::backgroundMediaId() {
  if (!hasAccountData(backgroundEventType))
    return {};

  auto url = backgroundUrl();
  return url.authority() + url.path();
}

void SpectralRoom::changeAvatar(QUrl localFile) {
  auto job = connection()->uploadFile(localFile.toLocalFile());
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

  aliases.append(alias);

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
  const char* tmp_buf =
      cmark_markdown_to_html(str.constData(), str.size(), CMARK_OPT_DEFAULT);

  std::string html(tmp_buf);

  free((char*)tmp_buf);

  auto result = QString::fromStdString(html).trimmed();

  result.replace("<p>", "");
  result.replace("</p>", "");

  return result;
}

void SpectralRoom::postArbitaryMessage(const QString& text,
                                       MessageEventType type,
                                       const QString& replyEventId) {
  auto parsedHTML = markdownToHTML(text);
  bool isRichText = Qt::mightBeRichText(parsedHTML);

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

    QJsonObject json{
        {"msgtype", msgTypeToString(type)},
        {"body", "> <" + replyEvt.senderId() + "> " + eventToString(replyEvt) +
                     "\n\n" + text},
        {"format", "org.matrix.custom.html"},
        {"m.relates_to",
         QJsonObject{
             {"m.in_reply_to", QJsonObject{{"event_id", replyEventId}}}}},
        {"formatted_body",
         "<mx-reply><blockquote><a href=\"https://matrix.to/#/" + id() + "/" +
             replyEventId +
             "\">In reply to</a> <a href=\"https://matrix.to/#/" +
             replyEvt.senderId() + "\">" + replyEvt.senderId() + "</a><br>" +
             utils::removeReply(eventToString(replyEvt, Qt::RichText)) +
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

    QJsonObject json{
        {"msgtype", msgTypeToString(type)},
        {"body", "> <" + replyEvt.senderId() + "> " + eventToString(replyEvt) +
                     "\n\n" + text},
        {"format", "org.matrix.custom.html"},
        {"m.relates_to",
         QJsonObject{
             {"m.in_reply_to", QJsonObject{{"event_id", replyEventId}}}}},
        {"formatted_body",
         "<mx-reply><blockquote><a href=\"https://matrix.to/#/" + id() + "/" +
             replyEventId +
             "\">In reply to</a> <a href=\"https://matrix.to/#/" +
             replyEvt.senderId() + "\">" + replyEvt.senderId() + "</a><br>" +
             utils::removeReply(eventToString(replyEvt, Qt::RichText)) +
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
    for (auto redactEventId : redactEventIds) {
      redactEvent(redactEventId);
    }
  } else {
    postReaction(eventId, reaction);
  }
}
