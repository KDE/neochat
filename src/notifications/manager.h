#pragma once

#include <QImage>
#include <QMap>
#include <QObject>
#include <QString>
#include <QUrl>

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusInterface>
#endif

struct roomEventId {
  QString roomId;
  QString eventId;
};

class NotificationsManager : public QObject {
  Q_OBJECT
 public:
  NotificationsManager(QObject* parent = nullptr);

 signals:
  void notificationClicked(const QString roomId, const QString eventId);

 private:
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
  QDBusInterface dbus;
  uint showNotification(const QString summary,
                        const QString text,
                        const QImage image);
#endif

  // notification ID to (room ID, event ID)
  QMap<uint, roomEventId> notificationIds;

  // these slots are platform specific (D-Bus only)
  // but Qt slot declarations can not be inside an ifdef!
 public slots:
  void actionInvoked(uint id, QString action);
  void notificationClosed(uint id, uint reason);

  void postNotification(const QString& roomId,
                        const QString& eventId,
                        const QString& roomName,
                        const QString& senderName,
                        const QString& text,
                        const QImage& icon);
};

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
QDBusArgument& operator<<(QDBusArgument& arg, const QImage& image);
const QDBusArgument& operator>>(const QDBusArgument& arg, QImage&);
#endif
