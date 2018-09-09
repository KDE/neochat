#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "connection.h"
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

  Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)

 public:
  explicit Controller(QObject* parent = nullptr);
  ~Controller();

  // All the Q_INVOKABLEs.
  Q_INVOKABLE void loginWithCredentials(QString, QString, QString);

  QVector<Connection*> connections() { return m_connections; }

  // All the non-Q_INVOKABLE functions.
  void addConnection(Connection* c);
  void dropConnection(Connection* c);

  // All the Q_PROPERTYs.
  bool busy() { return m_busy; }
  void setBusy(bool value) {
    if (value != m_busy) {
      m_busy = value;
      emit busyChanged();
    }
  }

  QVector<Connection*> m_connections;

 private:
  QClipboard* m_clipboard = QApplication::clipboard();
  QSystemTrayIcon* tray = new QSystemTrayIcon();
  QMenu* trayMenu = new QMenu();

  bool m_busy = false;

  void connected();
  void resync();
  void reconnect();
  QByteArray loadAccessToken(const AccountSettings& account);
  bool saveAccessToken(const AccountSettings& account,
                       const QByteArray& accessToken);
  void loadSettings();
  void saveSettings() const;

 private slots:
  void invokeLogin();

 signals:
  void busyChanged();
  void errorOccured();
  void toggleWindow();
  void connectionAdded(Connection* conn);
  void connectionDropped(Connection* conn);

 public slots:
  void joinRoom(const QString& alias);
  void createRoom(const QString& name, const QString& topic);
  void createDirectChat(const QString& userID);
  void copyToClipboard(const QString& text);
  void playAudio(QUrl localFile);
  void showMessage(const QString& title, const QString& msg, const QIcon& icon);
};

#endif  // CONTROLLER_H
