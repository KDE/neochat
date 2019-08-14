#include "manager.h"

#import <UserNotifications/UserNotifications.h>

#include <QApplication>

NotificationsManager::NotificationsManager(QObject* parent) : QObject(parent) {

}

void NotificationsManager::postNotification(const QString& roomId,
                                            const QString& eventId,
                                            const QString& roomName,
                                            const QString& senderName,
                                            const QString& text,
                                            const QImage& icon) {
  Q_UNUSED(roomId);
  Q_UNUSED(eventId);
  Q_UNUSED(icon);

  UNUserNotificationCenter* center =
      [UNUserNotificationCenter currentNotificationCenter];
  UNAuthorizationOptions options = UNAuthorizationOptionAlert + UNAuthorizationOptionSound;

  [center requestAuthorizationWithOptions:options
   completionHandler:^(BOOL granted, NSError * _Nullable error) {
    if (!granted) {
      NSLog(@"Something went wrong");
    }
  }];

  UNTimeIntervalNotificationTrigger *trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:300
                                    repeats:NO];

  UNMutableNotificationContent *content = [UNMutableNotificationContent new];

  content.title = roomName.toNSString();
  content.subtitle = QString("%1 sent a message").arg(senderName).toNSString();
  content.body = text.toNSString();
  content.sound = [UNNotificationSound defaultSound];

  NSString *identifier = QApplication::applicationName().toNSString();

  UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:identifier
                                    content:content trigger:trigger];

  [center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
      if (error != nil) {
          NSLog(@"Something went wrong: %@",error);
      }
  }];
}

// unused
void NotificationsManager::actionInvoked(uint, QString) {}

void NotificationsManager::notificationClosed(uint, uint) {}
