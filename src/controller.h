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
  Q_PROPERTY(int accountCount READ accountCount NOTIFY connectionAdded NOTIFY
                 connectionDropped)

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
  int accountCount() { return m_connections.count(); }

  Q_INVOKABLE QColor color(QString userId);
  Q_INVOKABLE void setColor(QString userId, QColor newColor);

 private:
  QClipboard* m_clipboard = QApplication::clipboard();
  QSystemTrayIcon* tray = new QSystemTrayIcon();
  QMenu* trayMenu = new QMenu();
  QVector<Connection*> m_connections;

  QByteArray loadAccessToken(const AccountSettings& account);
  bool saveAccessToken(const AccountSettings& account,
                       const QByteArray& accessToken);
  void loadSettings();
  void saveSettings() const;

 private slots:
  void invokeLogin();

 signals:
  void busyChanged();
  void errorOccured(QString error, QString detail);
  void showWindow();
  void hideWindow();
  void connectionAdded(Connection* conn);
  void connectionDropped(Connection* conn);
  void initiated();

 public slots:
  void logout(Connection* conn);
  void joinRoom(Connection* c, const QString& alias);
  void createRoom(Connection* c, const QString& name, const QString& topic);
  void createDirectChat(Connection* c, const QString& userID);
  void copyToClipboard(const QString& text);
  void playAudio(QUrl localFile);
  void showMessage(const QString& title, const QString& msg, const QIcon& icon);

  static QImage safeImage(QImage image);
};

#endif  // CONTROLLER_H
