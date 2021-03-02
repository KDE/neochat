// SPDX-FileCopyrightText: 2021 Contributors
// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <utility>
#include <vector>

#include <QObject>

typedef struct _GstDevice GstDevice;

class CallDevices;
class AudioDevicesModel;
class VideoDevicesModel;

class CallDevices : public QObject
{
    Q_OBJECT

    Q_PROPERTY(AudioDevicesModel *audioDevices READ audioDevicesModel CONSTANT);
    Q_PROPERTY(VideoDevicesModel *videoDevices READ videoDevicesModel CONSTANT);

public:
    static CallDevices &instance()
    {
        static CallDevices instance;
        return instance;
    }
    CallDevices(CallDevices const &) = delete;
    void operator=(CallDevices const &) = delete;

    bool hasMicrophone() const;
    bool hasCamera() const;
    QStringList names(bool isVideo, const QString &defaultDevice) const;
    QStringList resolutions(const QString &cameraName) const;
    QStringList frameRates(const QString &cameraName, const QString &resolution) const;

    AudioDevicesModel *audioDevicesModel() const;
    VideoDevicesModel *videoDevicesModel() const;

    void addDevice(GstDevice *device);
    void removeDevice(GstDevice *device, bool changed);

Q_SIGNALS:
    void devicesChanged();

private:
    CallDevices();

    void init();
    GstDevice *audioDevice() const;
    GstDevice *videoDevice(QPair<int, int> &resolution, QPair<int, int> &frameRate) const;

    AudioDevicesModel *m_audioDevicesModel;
    VideoDevicesModel *m_videoDevicesModel;

    friend class CallSession;
    friend class Audio;
};
