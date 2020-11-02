#include "notificationsmanager.h"

#include <QDebug>
#include <QImage>

#include <KNotification>

NotificationsManager::NotificationsManager(QObject *parent)
    : QObject(parent)
{
}

void NotificationsManager::postNotification(const QString &roomid, const QString &eventid, const QString &roomname, const QString &sender, const QString &text, const QImage &icon)
{
    QPixmap img;
    img.convertFromImage(icon);
    KNotification *notification = new KNotification("message");
    notification->setTitle(i18n("%1 (%2)", sender, roomname));
    notification->setText(text);
    notification->setPixmap(img);
    notification->sendEvent();
    
    m_notifications.insert(roomid, notification);
}
