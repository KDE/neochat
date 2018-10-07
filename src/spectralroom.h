#ifndef SpectralRoom_H
#define SpectralRoom_H

#include "room.h"

#include <QObject>
#include <QTimer>

using namespace QMatrixClient;

class SpectralRoom : public Room {
  Q_OBJECT
  Q_PROPERTY(QImage avatar READ getAvatar NOTIFY avatarChanged)
  Q_PROPERTY(bool hasUsersTyping READ hasUsersTyping NOTIFY typingChanged)
  Q_PROPERTY(QString usersTyping READ getUsersTyping NOTIFY typingChanged)
  Q_PROPERTY(QString cachedInput READ cachedInput WRITE setCachedInput NOTIFY
                 cachedInputChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

 public:
  explicit SpectralRoom(Connection* connection, QString roomId,
                        JoinState joinState = {});

  QImage getAvatar() { return avatar(128); }

  const QString& cachedInput() const { return m_cachedInput; }
  void setCachedInput(const QString& input) {
    if (input != m_cachedInput) {
      m_cachedInput = input;
      emit cachedInputChanged();
    }
  }

  bool busy() { return m_busy; }
  void setBusy(bool value) {
    if (m_busy != value) {
      m_busy = value;
      emit busyChanged();
    }
  }

  bool hasUsersTyping();
  QString getUsersTyping();

  QString lastEvent();
  bool isEventHighlighted(const QMatrixClient::RoomEvent* e) const;

  QDateTime lastActiveTime();

  Q_INVOKABLE float orderForTag(QString name);
  Q_INVOKABLE int savedTopVisibleIndex() const;
  Q_INVOKABLE int savedBottomVisibleIndex() const;
  Q_INVOKABLE void saveViewport(int topIndex, int bottomIndex);

 private:
  QString m_cachedInput;
  QSet<const QMatrixClient::RoomEvent*> highlights;

  bool m_busy;

  QString getMIME(const QUrl& fileUrl) const;
  void postFile(const QUrl& localFile, const QUrl& mxcUrl);

  void checkForHighlights(const QMatrixClient::TimelineItem& ti);

  void onAddNewTimelineEvents(timeline_iter_t from) override;
  void onAddHistoricalTimelineEvents(rev_iter_t from) override;

 private slots:
  void countChanged();

 signals:
  void cachedInputChanged();
  void busyChanged();

 public slots:
  void chooseAndUploadFile();
  void saveFileAs(QString eventId);
  void acceptInvitation();
  void forget();
  void sendTypingNotification(bool isTyping);
  void sendReply(QString userId, QString eventId, QString replyContent,
                 QString sendContent);
};

#endif  // SpectralRoom_H
