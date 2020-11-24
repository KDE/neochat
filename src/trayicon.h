/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef TRAYICON_H
#define TRAYICON_H

// Modified from mujx/nheko's TrayIcon.

#include <QAction>
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QRect>
#include <KStatusNotifierItem>

class TrayIcon : public KStatusNotifierItem
{
    Q_OBJECT
    Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged)
    Q_PROPERTY(bool isOnline READ isOnline WRITE setIsOnline NOTIFY isOnlineChanged)
public:
    TrayIcon(QObject *parent = nullptr);

    QString iconSource()
    {
        return m_iconSource;
    }
    void setIconSource(const QString &source);

    bool isOnline()
    {
        return m_isOnline;
    }
    void setIsOnline(bool online);

Q_SIGNALS:
    void notificationCountChanged();
    void iconSourceChanged();
    void isOnlineChanged();

    void showWindow();

private:
    QString m_iconSource;
    bool m_isOnline = true;
};

#endif // TRAYICON_H
