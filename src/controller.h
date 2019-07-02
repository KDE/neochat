#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "connection.h"
#include "notifications/manager.h"
#include "room.h"
#include "settings.h"
#include "user.h"

#include <QApplication>
#include <QMediaPlayer>
#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>

using namespace QMatrixClient;

class Controller : public QObject {
  Q_OBJECT
  Q_PROPERTY(int accountCount READ accountCount NOTIFY connectionAdded NOTIFY
                 connectionDropped)
  Q_PROPERTY(bool quitOnLastWindowClosed READ quitOnLastWindowClosed WRITE
                 setQuitOnLastWindowClosed NOTIFY quitOnLastWindowClosedChanged)
  Q_PROPERTY(Connection* connection READ connection WRITE setConnection NOTIFY
                 connectionChanged)
  Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)

 public:
  explicit Controller(QObject* parent = nullptr);
  ~Controller();

  // All the Q_INVOKABLEs.
  Q_INVOKABLE void loginWithCredentials(QString, QString, QString);

  QVector<Connection*> connections() { return m_connections; }

  Q_INVOKABLE int dpi();
  Q_INVOKABLE void setDpi(int dpi);

  // All the non-Q_INVOKABLE functions.
  void addConnection(Connection* c);
  void dropConnection(Connection* c);

  // All the Q_PROPERTYs.
  int accountCount() { return m_connections.count(); }

  bool quitOnLastWindowClosed() {
    return QApplication::quitOnLastWindowClosed();
  }
  void setQuitOnLastWindowClosed(bool value) {
    if (quitOnLastWindowClosed() != value) {
      QApplication::setQuitOnLastWindowClosed(value);
      emit quitOnLastWindowClosedChanged();
    }
  }

  bool busy() { return m_busy; }
  void setBusy(bool busy) {
    m_busy = busy;
    emit busyChanged();
  }

  Connection* connection() {
    if (m_connection.isNull())
      return nullptr;
    return m_connection;
  }

  void setConnection(Connection* conn) {
    if (conn == m_connection)
      return;
    m_connection = conn;
    emit connectionChanged();
  }

 private:
  QVector<Connection*> m_connections;
  QPointer<Connection> m_connection;
  bool m_busy = false;

  QByteArray loadAccessTokenFromFile(const AccountSettings& account);
  QByteArray loadAccessTokenFromKeyChain(const AccountSettings& account);

  bool saveAccessTokenToFile(const AccountSettings& account,
                             const QByteArray& accessToken);
  bool saveAccessTokenToKeyChain(const AccountSettings& account,
                                 const QByteArray& accessToken);
  void loadSettings();
  void saveSettings() const;

 private slots:
  void invokeLogin();

 signals:
  void busyChanged();
  void errorOccured(QString error, QString detail);
  void syncDone();
  void connectionAdded(Connection* conn);
  void connectionDropped(Connection* conn);
  void initiated();
  void notificationClicked(const QString roomId, const QString eventId);
  void quitOnLastWindowClosedChanged();
  void unreadCountChanged();
  void connectionChanged();

 public slots:
  void logout(Connection* conn);
  void joinRoom(Connection* c, const QString& alias);
  void createRoom(Connection* c, const QString& name, const QString& topic);
  void createDirectChat(Connection* c, const QString& userID);
  void playAudio(QUrl localFile);
};

#endif  // CONTROLLER_H
