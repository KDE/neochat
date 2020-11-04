#pragma once

#include <QImage>
#include <QMap>
#include <QObject>
#include <QString>

#include <KNotification>

class NotificationsManager : public QObject
{
    Q_OBJECT

public:
    NotificationsManager(QObject *parent = nullptr);

    Q_INVOKABLE void postNotification(const QString &roomId, const QString &eventId, const QString &roomName, const QString &senderName, const QString &text, const QImage &icon);

private:
    QMultiMap<QString, KNotification *> m_notifications;
};
