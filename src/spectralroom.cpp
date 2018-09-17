#include "spectralroom.h"

#include "connection.h"
#include "user.h"

#include "csapi/leaving.h"
#include "csapi/typing.h"
#include "events/typingevent.h"

#include <QFileDialog>
#include <QMimeDatabase>

SpectralRoom::SpectralRoom(Connection* connection, QString roomId,
                           JoinState joinState)
    : Room(connection, std::move(roomId), joinState) {
  connect(this, &SpectralRoom::notificationCountChanged, this,
          &SpectralRoom::countChanged);
  connect(this, &SpectralRoom::highlightCountChanged, this,
          &SpectralRoom::countChanged);
}

void SpectralRoom::chooseAndUploadFile() {
  auto localFile = QFileDialog::getOpenFileUrl(Q_NULLPTR, tr("Save File as"));
  if (!localFile.isEmpty()) {
    uploadFile(localFile.toString(), localFile, getMIME(localFile));
    QMetaObject::Connection* const connection = new QMetaObject::Connection;
    *connection = connect(this, &Room::fileTransferCompleted,
                          [=](QString id, QUrl localFile, QUrl mxcUrl) {
                            disconnect(*connection);
                            postFile(localFile, mxcUrl);
                          });
  }
}

void SpectralRoom::postFile(const QUrl& localFile, const QUrl& mxcUrl) {
  const QString mime = getMIME(localFile);
  const QString fileName = localFile.fileName();
  QString msgType = "m.file";
  if (mime.startsWith("image")) msgType = "m.image";
  if (mime.startsWith("video")) msgType = "m.video";
  if (mime.startsWith("audio")) msgType = "m.audio";
  QJsonObject json{QJsonObject{{"msgtype", msgType},
                               {"body", fileName},
                               {"filename", fileName},
                               {"url", mxcUrl.url()}}};
  postJson("m.room.message", json);
}

QString SpectralRoom::getMIME(const QUrl& fileUrl) const {
  return QMimeDatabase().mimeTypeForFile(fileUrl.toLocalFile()).name();
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

QString SpectralRoom::getUsersTyping() {
  QString usersTypingStr;
  QList<User*> users = usersTyping();
  users.removeOne(localUser());
  for (User* user : users) {
    usersTypingStr += user->displayname() + " ";
  }
  usersTypingStr += users.count() < 2 ? "is" : "are";
  usersTypingStr += " typing.";
  return usersTypingStr;
}

void SpectralRoom::sendTypingNotification(bool isTyping) {
  connection()->callApi<SetTypingJob>(BackgroundRequest, localUser()->id(),
                                      id(), isTyping, 10000);
}

QString SpectralRoom::lastEvent() {
  if (timelineSize() == 0) return "";
  const RoomEvent* lastEvent = messageEvents().rbegin()->get();
  if (lastEvent->contentJson().value("body").toString() == "") return "";
  return user(lastEvent->senderId())->displayname() + ": " +
         lastEvent->contentJson().value("body").toString();
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
