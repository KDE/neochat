#include "matriqueroom.h"

#include "connection.h"
#include "user.h"

#include "csapi/leaving.h"
#include "csapi/typing.h"
#include "events/typingevent.h"

#include <QFileDialog>
#include <QMimeDatabase>

MatriqueRoom::MatriqueRoom(Connection* connection, QString roomId,
                           JoinState joinState)
    : Room(connection, std::move(roomId), joinState) {
  connect(this, &MatriqueRoom::notificationCountChanged, this,
          &MatriqueRoom::countChanged);
  connect(this, &MatriqueRoom::highlightCountChanged, this,
          &MatriqueRoom::countChanged);
}

void MatriqueRoom::chooseAndUploadFile() {
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

void MatriqueRoom::postFile(const QUrl& localFile, const QUrl& mxcUrl) {
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

QString MatriqueRoom::getMIME(const QUrl& fileUrl) const {
  return QMimeDatabase().mimeTypeForFile(fileUrl.toLocalFile()).name();
}

void MatriqueRoom::saveFileAs(QString eventId) {
  auto fileName = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save File as"),
                                               fileNameToDownload(eventId));
  if (!fileName.isEmpty()) downloadFile(eventId, QUrl::fromLocalFile(fileName));
}

void MatriqueRoom::acceptInvitation() { connection()->joinRoom(id()); }

void MatriqueRoom::forget() { connection()->forgetRoom(id()); }

bool MatriqueRoom::hasUsersTyping() {
  QList<User*> users = usersTyping();
  if (users.isEmpty()) return false;
  int count = users.length();
  if (users.contains(localUser())) count--;
  return count != 0;
}

QString MatriqueRoom::getUsersTyping() {
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

void MatriqueRoom::sendTypingNotification(bool isTyping) {
  connection()->callApi<SetTypingJob>(BackgroundRequest, localUser()->id(),
                                      id(), isTyping, 10000);
}

QString MatriqueRoom::lastEvent() {
  if (timelineSize() == 0) return "";
  const RoomEvent* lastEvent = messageEvents().rbegin()->get();
  if (lastEvent->contentJson().value("body").toString() == "") return "";
  return user(lastEvent->senderId())->displayname() + ": " +
         lastEvent->contentJson().value("body").toString();
}

bool MatriqueRoom::isEventHighlighted(const RoomEvent* e) const {
  return highlights.contains(e);
}

void MatriqueRoom::checkForHighlights(const QMatrixClient::TimelineItem& ti) {
  auto localUserId = localUser()->id();
  if (ti->senderId() == localUserId) return;
  if (auto* e = ti.viewAs<RoomMessageEvent>()) {
    const auto& text = e->plainBody();
    if (text.contains(localUserId) ||
        text.contains(roomMembername(localUserId)))
      highlights.insert(e);
  }
}

void MatriqueRoom::onAddNewTimelineEvents(timeline_iter_t from) {
  std::for_each(from, messageEvents().cend(),
                [this](const TimelineItem& ti) { checkForHighlights(ti); });
}

void MatriqueRoom::onAddHistoricalTimelineEvents(rev_iter_t from) {
  std::for_each(from, messageEvents().crend(),
                [this](const TimelineItem& ti) { checkForHighlights(ti); });
}

void MatriqueRoom::countChanged() {
  if (displayed() && !hasUnreadMessages()) {
    resetNotificationCount();
    resetHighlightCount();
  }
}
