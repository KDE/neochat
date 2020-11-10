/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "trayicon.h"

// Modified from mujx/nheko's TrayIcon.

#include <QApplication>
#include <QList>
#include <QMenu>
#include <QTimer>
#include <QtDebug>

#if defined(Q_OS_MAC)
#include <QtMacExtras>
#endif

MsgCountComposedIcon::MsgCountComposedIcon(const QString &filename)
    : QIconEngine()
{
    icon_ = QIcon(filename);
}

void MsgCountComposedIcon::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::Antialiasing);

    icon_.paint(painter, rect, Qt::AlignCenter, mode, state);

    if (isOnline && msgCount <= 0)
        return;

    QColor backgroundColor("red");
    QColor textColor("white");

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(backgroundColor);

    painter->setBrush(brush);
    painter->setPen(Qt::NoPen);
    painter->setFont(QFont("Open Sans", 8, QFont::Black));

    QRectF bubble(rect.width() - BubbleDiameter, rect.height() - BubbleDiameter, BubbleDiameter, BubbleDiameter);
    painter->drawEllipse(bubble);
    painter->setPen(QPen(textColor));
    painter->setBrush(Qt::NoBrush);
    if (!isOnline) {
        painter->drawText(bubble, Qt::AlignCenter, "x");
    } else if (msgCount >= 100) {
        painter->drawText(bubble, Qt::AlignCenter, "99+");
    } else {
        painter->drawText(bubble, Qt::AlignCenter, QString::number(msgCount));
    }
}

QIconEngine *MsgCountComposedIcon::clone() const
{
    return new MsgCountComposedIcon(*this);
}

QList<QSize> MsgCountComposedIcon::availableSizes(QIcon::Mode mode, QIcon::State state) const
{
    Q_UNUSED(mode)
    Q_UNUSED(state)
    QList<QSize> sizes;
    sizes.append(QSize(24, 24));
    sizes.append(QSize(32, 32));
    sizes.append(QSize(48, 48));
    sizes.append(QSize(64, 64));
    sizes.append(QSize(128, 128));
    sizes.append(QSize(256, 256));
    return sizes;
}

QPixmap MsgCountComposedIcon::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QImage img(size, QImage::Format_ARGB32);
    img.fill(qRgba(0, 0, 0, 0));
    QPixmap result = QPixmap::fromImage(img, Qt::NoFormatConversion);
    {
        QPainter painter(&result);
        paint(&painter, QRect(QPoint(0, 0), size), mode, state);
    }
    return result;
}

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
{
    QMenu *menu = new QMenu();
    viewAction_ = new QAction(tr("Show"), parent);
    quitAction_ = new QAction(tr("Quit"), parent);

    connect(viewAction_, &QAction::triggered, this, &TrayIcon::showWindow);
    connect(quitAction_, &QAction::triggered, this, QApplication::quit);

    menu->addAction(viewAction_);
    menu->addAction(quitAction_);

    setContextMenu(menu);
}

void TrayIcon::setNotificationCount(int count)
{
    m_notificationCount = count;
// Use the native badge counter in MacOS.
#if defined(Q_OS_MAC)
    auto labelText = count == 0 ? "" : QString::number(count);

    if (labelText == QtMac::badgeLabelText())
        return;

    QtMac::setBadgeLabelText(labelText);
#elif defined(Q_OS_WIN)
// FIXME: Find a way to use Windows apis for the badge counter (if any).
#else
    if (!icon_ || count == icon_->msgCount)
        return;

    // Custom drawing on Linux.
    MsgCountComposedIcon *tmp = static_cast<MsgCountComposedIcon *>(icon_->clone());
    tmp->msgCount = count;

    setIcon(QIcon(tmp));

    icon_ = tmp;
#endif
    Q_EMIT notificationCountChanged();
}

void TrayIcon::setIsOnline(bool online)
{
    m_isOnline = online;
#if defined(Q_OS_MAC)
    if (online) {
        auto labelText = m_notificationCount == 0 ? "" : QString::number(m_notificationCount);

        if (labelText == QtMac::badgeLabelText())
            return;

        QtMac::setBadgeLabelText(labelText);
    } else {
        auto labelText = "x";

        if (labelText == QtMac::badgeLabelText())
            return;

        QtMac::setBadgeLabelText(labelText);
    }
#elif defined(Q_OS_WIN)
// FIXME: Find a way to use Windows apis for the badge counter (if any).
#else
    if (!icon_ || online == icon_->isOnline)
        return;

    // Custom drawing on Linux.
    MsgCountComposedIcon *tmp = static_cast<MsgCountComposedIcon *>(icon_->clone());
    tmp->isOnline = online;

    setIcon(QIcon(tmp));

    icon_ = tmp;
#endif
    Q_EMIT isOnlineChanged();
}

void TrayIcon::setIconSource(const QString &source)
{
    m_iconSource = source;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    setIcon(QIcon(source));
#else
    icon_ = new MsgCountComposedIcon(source);
    setIcon(QIcon(icon_));
    icon_->isOnline = m_isOnline;
    icon_->msgCount = m_notificationCount;
#endif
    Q_EMIT iconSourceChanged();
}
