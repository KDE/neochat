// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QVector>

#include <gst/gst.h>

struct AudioSource {
    QString title;
    GstDevice *device;
    bool isDefault;
};
struct VideoCap {
    int width;
    int height;
    QVector<float> framerates;
};

struct VideoSource {
    QString title;
    GstDevice *device;
    QVector<VideoCap> caps;
};

class DeviceMonitor : public QObject
{
    Q_OBJECT

public:
    static DeviceMonitor &instance()
    {
        static DeviceMonitor _instance;
        return _instance;
    }

    QVector<AudioSource *> audioSources() const;
    QVector<VideoSource *> videoSources() const;
    bool callback(GstMessage *message);
    void init();

Q_SIGNALS:
    void videoSourceAdded();
    void audioSourceAdded();

    void videoSourceRemoved();
    void audioSourceRemoved();

private:
    DeviceMonitor();
    GstDeviceMonitor *m_monitor = nullptr;
    QVector<AudioSource *> m_audioSources;
    QVector<VideoSource *> m_videoSources;
    void handleVideoSource(GstDevice *device);
    void handleAudioSource(GstDevice *device);
};
