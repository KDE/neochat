#ifndef TRAYICON_H
#define TRAYICON_H

// Modified from mujx/nheko's TrayIcon.

#include <QAction>
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QRect>
#include <QSystemTrayIcon>

class MsgCountComposedIcon : public QIconEngine
{
public:
    MsgCountComposedIcon(const QString &filename);

    virtual void paint(QPainter *p, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    virtual QIconEngine *clone() const override;
    virtual QList<QSize> availableSizes(QIcon::Mode mode, QIcon::State state) const override;
    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;

    int msgCount = 0;
    bool isOnline = true; // Default to false?

private:
    const int BubbleDiameter = 14;

    QIcon icon_;
};

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
    Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged)
    Q_PROPERTY(int notificationCount READ notificationCount WRITE setNotificationCount NOTIFY notificationCountChanged)
    Q_PROPERTY(bool isOnline READ isOnline WRITE setIsOnline NOTIFY isOnlineChanged)
public:
    TrayIcon(QObject *parent = nullptr);

    QString iconSource()
    {
        return m_iconSource;
    }
    void setIconSource(const QString &source);

    int notificationCount()
    {
        return m_notificationCount;
    }
    void setNotificationCount(int count);

    bool isOnline()
    {
        return m_isOnline;
    }
    void setIsOnline(bool online);

signals:
    void notificationCountChanged();
    void iconSourceChanged();
    void isOnlineChanged();

    void showWindow();

private:
    QString m_iconSource;
    int m_notificationCount = 0;
    bool m_isOnline = true;

    QAction *viewAction_;
    QAction *quitAction_;

    MsgCountComposedIcon *icon_ = nullptr;
};

#endif // TRAYICON_H
