/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
    static NotificationsManager &instance();

    Q_INVOKABLE void postNotification(const QString &roomId, const QString &eventId, const QString &roomName, const QString &senderName, const QString &text, const QImage &icon);

private:
    NotificationsManager(QObject *parent = nullptr);

    QMultiMap<QString, KNotification *> m_notifications;
};
