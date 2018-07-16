#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "connection.h"
#include "roomlistmodel.h"
#include "user.h"

namespace QMatrixClient {
class Connection;
}

class Controller : public QObject {
  Q_OBJECT

  Q_PROPERTY(QMatrixClient::Connection* connection READ getConnection CONSTANT)
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
  QMatrixClient::Connection* m_connection = new QMatrixClient::Connection();
  QMatrixClient::Connection* getConnection() { return m_connection; }

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
  void postFile(QMatrixClient::Room* room, const QUrl& localFile,
                const QUrl& mxcUrl);
};

#endif  // CONTROLLER_H
