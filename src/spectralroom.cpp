#include "spectralroom.h"

#include "connection.h"
#include "user.h"

#include "csapi/content-repo.h"
#include "csapi/leaving.h"
#include "csapi/typing.h"
#include "events/typingevent.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMetaObject>
#include <QMimeDatabase>

#include "html.h"

#include "utils.h"

SpectralRoom::SpectralRoom(Connection* connection, QString roomId,
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
}

inline QString getMIME(const QUrl& fileUrl) {
  return QMimeDatabase().mimeTypeForFile(fileUrl.toLocalFile()).name();
}

inline QSize getImageSize(const QUrl& imageUrl) {
  QImageReader reader(imageUrl.toLocalFile());
  return reader.size();
}

void SpectralRoom::chooseAndUploadFile() {
  auto localFile = QFileDialog::getOpenFileUrl(Q_NULLPTR, tr("Save File as"));
  if (!localFile.isEmpty()) {
    QString txnID = postFile(localFile.fileName(), localFile, false);
    setHasFileUploading(true);
    connect(this, &Room::fileTransferCompleted,
            [=](QString id, QUrl localFile, QUrl mxcUrl) {
              if (id == txnID) {
                setFileUploadingProgress(0);
                setHasFileUploading(false);
              }
            });
    connect(this, &Room::fileTransferFailed, [=](QString id, QString error) {
      if (id == txnID) {
        setFileUploadingProgress(0);
        setHasFileUploading(false);
      }
    });
    connect(
        this, &Room::fileTransferProgress,
        [=](QString id, qint64 progress, qint64 total) {
          if (id == txnID) {
            qDebug() << "Progress:" << progress << total;
            setFileUploadingProgress(int(float(progress) / float(total) * 100));
          }
        });
  }
}

void SpectralRoom::saveFileAs(QString eventId) {
  auto fileName = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save File as"),
                                               fileNameToDownload(eventId));
  if (!fileName.isEmpty()) downloadFile(eventId, QUrl::fromLocalFile(fileName));
}

void SpectralRoom::acceptInvitation() { connection()->joinRoom(id()); }

void SpectralRoom::forget() { connection()->forgetRoom(id()); }

bool SpectralRoom::hasUsersTyping() {
  QList<User*> users = usersTyping();
  if (users.isEmpty()) return false;
  int count = users.length();
  if (users.contains(localUser())) count--;
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
  if (timelineSize() == 0) return "";
  const RoomEvent* lastEvent = messageEvents().rbegin()->get();
  return user(lastEvent->senderId())->displayname() + ": " +
         utils::removeReply(eventToString(*lastEvent));
}

bool SpectralRoom::isEventHighlighted(const RoomEvent* e) const {
  return highlights.contains(e);
}

void SpectralRoom::checkForHighlights(const QMatrixClient::TimelineItem& ti) {
  auto localUserId = localUser()->id();
  if (ti->senderId() == localUserId) return;
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

void SpectralRoom::sendReply(QString userId, QString eventId,
                             QString replyContent, QString sendContent) {
  QJsonObject json{
      {"msgtype", "m.text"},
      {"body", "> <" + userId + "> " + replyContent + "\n\n" + sendContent},
      {"format", "org.matrix.custom.html"},
      {"m.relates_to",
       QJsonObject{{"m.in_reply_to", QJsonObject{{"event_id", eventId}}}}},
      {"formatted_body",
       "<mx-reply><blockquote><a href=\"https://matrix.to/#/" + id() + "/" +
           eventId + "\">In reply to</a> <a href=\"https://matrix.to/#/" +
           userId + "\">" + userId + "</a><br>" + replyContent +
           "</blockquote></mx-reply>" + sendContent}};
  postJson("m.room.message", json);
}

QDateTime SpectralRoom::lastActiveTime() {
  if (timelineSize() == 0) return QDateTime();
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

QString SpectralRoom::postMarkdownText(const QString& markdown) {
  unsigned char *sequence = (unsigned char *) qstrdup(markdown.toUtf8().constData());
  qint64 length = strlen((char *) sequence);

  hoedown_renderer* renderer = hoedown_html_renderer_new(HOEDOWN_HTML_USE_XHTML, 32);
  hoedown_extensions extensions = (hoedown_extensions) ((HOEDOWN_EXT_BLOCK | HOEDOWN_EXT_SPAN | HOEDOWN_EXT_MATH_EXPLICIT) & ~HOEDOWN_EXT_QUOTE);
  hoedown_document* document = hoedown_document_new(renderer, extensions, 32);
  hoedown_buffer* html = hoedown_buffer_new(length);
  hoedown_document_render(document, html, sequence, length);
  QString result = QString::fromUtf8((char *) html->data, html->size);

  free(sequence);
  hoedown_buffer_free(html);
  hoedown_document_free(document);
  hoedown_html_renderer_free(renderer);

  return postHtmlText(markdown, result);
}
