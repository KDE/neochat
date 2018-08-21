#ifndef MATRIQUEROOM_H
#define MATRIQUEROOM_H

#include "room.h"

#include <QObject>
#include <QTimer>

using namespace QMatrixClient;

class MatriqueRoom : public Room {
  Q_OBJECT
  Q_PROPERTY(
      bool isTyping READ isTyping WRITE setIsTyping NOTIFY isTypingChanged)
  Q_PROPERTY(bool hasUsersTyping READ hasUsersTyping NOTIFY typingChanged)
  Q_PROPERTY(QString usersTyping READ getUsersTyping NOTIFY typingChanged)
  Q_PROPERTY(QString cachedInput READ cachedInput WRITE setCachedInput NOTIFY
                 cachedInputChanged)
 public:
  explicit MatriqueRoom(Connection* connection, QString roomId,
                        JoinState joinState = {});

  bool isTyping() { return m_isTyping; }
  void setIsTyping(bool isTyping) {
    if (isTyping) m_timeoutTimer->start();
    if (isTyping != m_isTyping) {
      m_isTyping = isTyping;
      emit isTypingChanged();
    }
  }

  const QString& cachedInput() const { return m_cachedInput; }
  void setCachedInput(const QString& input) {
    if (input != m_cachedInput) {
      m_cachedInput = input;
      emit cachedInputChanged();
    }
  }

  bool hasUsersTyping();
  QString getUsersTyping();

 private:
  QString m_cachedInput;
  bool m_isTyping;
  QTimer* m_timeoutTimer = new QTimer();
  QTimer* m_repeatTimer = new QTimer();

  QString getMIME(const QUrl& fileUrl) const;
  void postFile(const QUrl& localFile, const QUrl& mxcUrl);

 signals:
  void isTypingChanged();
  void cachedInputChanged();

 public slots:
  void chooseAndUploadFile();
  void saveFileAs(QString eventId);
  void acceptInvitation();
  void forget();
  void sendTypingNotification(bool isTyping);
};

#endif  // MATRIQUEROOM_H
