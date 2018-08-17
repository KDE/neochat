#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "connection.h"
#include "roomlistmodel.h"
#include "user.h"

#include <QApplication>
#include <QMimeDatabase>
#include <QObject>

using namespace QMatrixClient;

class Controller : public QObject {
  Q_OBJECT

  Q_PROPERTY(Connection* connection READ getConnection CONSTANT)
  Q_PROPERTY(
      bool isLogin READ getIsLogin WRITE setIsLogin NOTIFY isLoginChanged)
  Q_PROPERTY(QString homeserver READ getHomeserver WRITE setHomeserver NOTIFY
                 homeserverChanged)
  Q_PROPERTY(QString userID READ getUserID WRITE setUserID NOTIFY userIDChanged)
  Q_PROPERTY(QByteArray token READ getToken WRITE setToken NOTIFY tokenChanged)
  Q_PROPERTY(bool busy READ getBusy WRITE setBusy NOTIFY busyChanged)

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
  Connection* getConnection() { return m_connection; }

  bool isLogin = false;
  bool getIsLogin() { return isLogin; }
  void setIsLogin(bool n) {
    if (n != isLogin) {
      isLogin = n;
      emit isLoginChanged();
    }
  }

  QString userID;
  QString getUserID() { return userID; }
  void setUserID(QString n) {
    if (n != userID) {
      userID = n;
      emit userIDChanged();
    }
  }

  QByteArray token;
  QByteArray getToken() { return token; }
  void setToken(QByteArray n) {
    if (n != token) {
      token = n;
      emit tokenChanged();
    }
  }

  QString homeserver;
  QString getHomeserver() { return homeserver; }
  void setHomeserver(QString n) {
    if (n != homeserver) {
      homeserver = n;
      emit homeserverChanged();
    }
  }

  bool busy = false;
  bool getBusy() { return busy; }
  void setBusy(bool b) {
    if (b != busy) {
      busy = b;
      emit busyChanged();
    }
  }

 private:
  QClipboard* m_clipboard = QApplication::clipboard();
  QMimeDatabase m_db;

  void connected();
  void resync();
  void reconnect();

  QString getMIME(const QUrl& fileUrl) const;
  void postFile(Room* room, const QUrl& localFile, const QUrl& mxcUrl);

 signals:
  void connectionChanged();
  void isLoginChanged();
  void userIDChanged();
  void tokenChanged();
  void homeserverChanged();
  void busyChanged();
  void errorOccured();

 public slots:
  void uploadFile(Room* room);
  void forgetRoom(const QString& roomID);
  void joinRoom(const QString& alias);
  void createRoom(const QString& name, const QString& topic);
  void createDirectChat(const QString& userID);
  void copyToClipboard(const QString& text);
  void saveFileAs(Room* room, QString eventId);
  void acceptRoom(Room* room);
  void rejectRoom(Room* room);
};

#endif  // CONTROLLER_H
