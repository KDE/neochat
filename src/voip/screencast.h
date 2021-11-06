// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtQuick/QQuickItem>

#include <gst/gst.h>

#include <optional>

/* Abstracts over screen sharing backends
 * <= 1 will be available
 * on some platforms (wayland), we can't select the window / screen
 */
class AbstractScreenCast : public QObject
{
    Q_OBJECT
public:
    virtual GstElement *request(int index) = 0;
    virtual bool canSelectWindow() const = 0;
    virtual bool canShareScreen() const = 0;
    AbstractScreenCast(QObject *parent);
};

class ScreenCastManager : public QObject
{
    Q_OBJECT
public:
    static ScreenCastManager &instance()
    {
        static ScreenCastManager _instance;
        return _instance;
    }

    Q_INVOKABLE void requestScreenCast(int index);

private:
    std::optional<AbstractScreenCast *> m_screencast;
    ScreenCastManager();
    QQuickItem *m_item;
};
