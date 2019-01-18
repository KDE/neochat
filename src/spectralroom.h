#ifndef SpectralRoom_H
#define SpectralRoom_H

#include "room.h"
#include "spectraluser.h"

#include <QObject>
#include <QPointer>
#include <QTimer>

using namespace QMatrixClient;

class SpectralRoom : public Room {
  Q_OBJECT
  Q_PROPERTY(bool hasUsersTyping READ hasUsersTyping NOTIFY typingChanged)
  Q_PROPERTY(QVariantList usersTyping READ getUsersTyping NOTIFY typingChanged)
  Q_PROPERTY(QString cachedInput READ cachedInput WRITE setCachedInput NOTIFY
                 cachedInputChanged)
  Q_PROPERTY(bool hasFileUploading READ hasFileUploading NOTIFY
                 hasFileUploadingChanged)
  Q_PROPERTY(int fileUploadingProgress READ fileUploadingProgress NOTIFY
                 fileUploadingProgressChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

 public:
  explicit SpectralRoom(Connection* connection, QString roomId,
                        JoinState joinState = {});

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
  QVariantList getUsersTyping();

  QString lastEvent();
  bool isEventHighlighted(const QMatrixClient::RoomEvent* e) const;

  QDateTime lastActiveTime();

  bool hasFileUploading() { return m_hasFileUploading; }
  void setHasFileUploading(bool value) {
    if (m_hasFileUploading != value) {
      m_hasFileUploading = value;
      emit hasFileUploadingChanged();
    }
  }

  int fileUploadingProgress() { return m_fileUploadingProgress; }
  void setFileUploadingProgress(int value) {
    if (m_fileUploadingProgress != value) {
      m_fileUploadingProgress = value;
      emit fileUploadingProgressChanged();
    }
  }

  Q_INVOKABLE int savedTopVisibleIndex() const;
  Q_INVOKABLE int savedBottomVisibleIndex() const;
  Q_INVOKABLE void saveViewport(int topIndex, int bottomIndex);

  Q_INVOKABLE void getPreviousContent(int limit = 10);

  Q_INVOKABLE QVariantList getUsers(const QString& prefix);

  Q_INVOKABLE QString postMarkdownText(const QString& markdown);

 private:
  QString m_cachedInput;
  QSet<const QMatrixClient::RoomEvent*> highlights;

  bool m_hasFileUploading = false;
  int m_fileUploadingProgress = 0;

  bool m_busy = false;

  void postFile(const QUrl& localFile, const QUrl& mxcUrl);

  void checkForHighlights(const QMatrixClient::TimelineItem& ti);

  void onAddNewTimelineEvents(timeline_iter_t from) override;
  void onAddHistoricalTimelineEvents(rev_iter_t from) override;

 private slots:
  void countChanged();

 signals:
  void cachedInputChanged();
  void busyChanged();
  void hasFileUploadingChanged();
  void fileUploadingProgressChanged();

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
