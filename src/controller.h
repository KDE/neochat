#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "connection.h"
#include "user.h"

#include <QApplication>
#include <QMediaPlayer>
#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

using namespace QMatrixClient;

class Controller : public QObject {
  Q_OBJECT

  Q_PROPERTY(Connection* connection READ connection CONSTANT)
  Q_PROPERTY(bool isLogin READ isLogin WRITE setIsLogin NOTIFY isLoginChanged)
  Q_PROPERTY(QString homeserver READ homeserver WRITE setHomeserver NOTIFY
                 homeserverChanged)
  Q_PROPERTY(QString userID READ userID WRITE setUserID NOTIFY userIDChanged)
  Q_PROPERTY(QByteArray token READ token WRITE setToken NOTIFY tokenChanged)
  Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)

 public:
  explicit Controller(QObject* parent = nullptr);
  ~Controller();

  // All the Q_INVOKABLEs.
  Q_INVOKABLE void login();
  Q_INVOKABLE void loginWithCredentials(QString, QString, QString);
  Q_INVOKABLE void logout();

  // All the non-Q_INVOKABLE functions.

  // All the Q_PROPERTYs.
  Connection* m_connection = new Connection();
  Connection* connection() { return m_connection; }

  bool isLogin() { return m_isLogin; }
  void setIsLogin(bool n) {
    if (n != m_isLogin) {
      m_isLogin = n;
      emit isLoginChanged();
    }
  }

  QString userID() { return m_userID; }
  void setUserID(QString n) {
    if (n != m_userID) {
      m_userID = n;
      emit userIDChanged();
    }
  }

  QByteArray token() { return m_token; }
  void setToken(QByteArray n) {
    if (n != m_token) {
      m_token = n;
      emit tokenChanged();
    }
  }

  QString homeserver() { return m_homeserver; }
  void setHomeserver(QString n) {
    if (n != m_homeserver) {
      m_homeserver = n;
      emit homeserverChanged();
    }
  }

  bool busy() { return m_busy; }
  void setBusy(bool b) {
    if (b != m_busy) {
      m_busy = b;
      emit busyChanged();
    }
  }

 private:
  QClipboard* m_clipboard = QApplication::clipboard();
  QSystemTrayIcon* tray = new QSystemTrayIcon();
  QMenu* trayMenu = new QMenu();

  bool m_isLogin = false;
  QString m_userID;
  QByteArray m_token;
  QString m_homeserver;
  bool m_busy = false;

  void connected();
  void resync();
  void reconnect();

 signals:
  void connectionChanged();
  void isLoginChanged();
  void userIDChanged();
  void tokenChanged();
  void homeserverChanged();
  void busyChanged();
  void errorOccured();

 public slots:
  void joinRoom(const QString& alias);
  void createRoom(const QString& name, const QString& topic);
  void createDirectChat(const QString& userID);
  void copyToClipboard(const QString& text);
  void playAudio(QUrl localFile);
  void showMessage(const QString& title, const QString& msg, const QIcon& icon);
};

#endif  // CONTROLLER_H
