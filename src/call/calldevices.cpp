// SPDX-FileCopyrightText: 2021 Nheko Contributors
// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "calldevices.h"
#include "audiodevicesmodel.h"
#include "neochatconfig.h"
#include "videodevicesmodel.h"
#include <QStringView>
#include <cstring>
#include <optional>

#include "voiplogging.h"

#ifdef GSTREAMER_AVAILABLE
extern "C" {
#include "gst/gst.h"
}
#endif

#ifdef GSTREAMER_AVAILABLE

CallDevices::CallDevices()
    : QObject()
    , m_audioDevicesModel(new AudioDevicesModel(this))
    , m_videoDevicesModel(new VideoDevicesModel(this))
{
    init();
}

AudioDevicesModel *CallDevices::audioDevicesModel() const
{
    return m_audioDevicesModel;
}

VideoDevicesModel *CallDevices::videoDevicesModel() const
{
    return m_videoDevicesModel;
}

void CallDevices::addDevice(GstDevice *device)
{
    if (!device)
        return;

    gchar *type = gst_device_get_device_class(device);
    bool isVideo = !std::strncmp(type, "Video", 5);
    g_free(type);
    if (isVideo) {
        m_videoDevicesModel->addDevice(device);
        m_videoDevicesModel->setDefaultDevice();
    } else {
        m_audioDevicesModel->addDevice(device);
        m_audioDevicesModel->setDefaultDevice();
    }
}

void CallDevices::removeDevice(GstDevice *device, bool changed)
{
    if (device) {
        if (m_audioDevicesModel->removeDevice(device, changed) || m_videoDevicesModel->removeDevice(device, changed))
            return;
    }
}

namespace
{
gboolean newBusMessage(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    Q_UNUSED(bus)
    Q_UNUSED(user_data)

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_DEVICE_ADDED: {
        GstDevice *device;
        gst_message_parse_device_added(msg, &device);
        CallDevices::instance().addDevice(device);
        Q_EMIT CallDevices::instance().devicesChanged();
        break;
    }
    case GST_MESSAGE_DEVICE_REMOVED: {
        GstDevice *device;
        gst_message_parse_device_removed(msg, &device);
        CallDevices::instance().removeDevice(device, false);
        Q_EMIT CallDevices::instance().devicesChanged();
        break;
    }
    case GST_MESSAGE_DEVICE_CHANGED: {
        GstDevice *device;
        GstDevice *oldDevice;
        gst_message_parse_device_changed(msg, &device, &oldDevice);
        CallDevices::instance().removeDevice(oldDevice, true);
        CallDevices::instance().addDevice(device);
        Q_EMIT CallDevices::instance().devicesChanged();
        break;
    }
    default:
        break;
    }
    return true;
}
}

void CallDevices::init()
{
    static GstDeviceMonitor *monitor = nullptr;
    if (!monitor) {
        monitor = gst_device_monitor_new();
        Q_ASSERT(monitor);
        GstCaps *caps = gst_caps_new_empty_simple("audio/x-raw");
        gst_device_monitor_add_filter(monitor, "Audio/Source", caps);
        gst_device_monitor_add_filter(monitor, "Audio/Duplex", caps);
        gst_caps_unref(caps);
        caps = gst_caps_new_empty_simple("video/x-raw");
        gst_device_monitor_add_filter(monitor, "Video/Source", caps);
        gst_device_monitor_add_filter(monitor, "Video/Duplex", caps);
        gst_caps_unref(caps);

        GstBus *bus = gst_device_monitor_get_bus(monitor);
        gst_bus_add_watch(bus, newBusMessage, nullptr);
        gst_object_unref(bus);
        if (!gst_device_monitor_start(monitor)) {
            qCCritical(voip) << "Failed to start device monitor";
            return;
        } else {
            qCDebug(voip) << "Device monitor started";
        }
    }
}

bool CallDevices::hasMicrophone() const
{
    return m_audioDevicesModel->hasMicrophone();
}

bool CallDevices::hasCamera() const
{
    return m_videoDevicesModel->hasCamera();
}

QStringList CallDevices::resolutions(const QString &cameraName) const
{
    return m_videoDevicesModel->resolutions(cameraName);
}

QStringList CallDevices::frameRates(const QString &cameraName, const QString &resolution) const
{
    if (auto s = m_videoDevicesModel->getVideoSource(cameraName); s) {
        if (auto it = std::find_if(s->caps.cbegin(),
                                   s->caps.cend(),
                                   [&](const auto &c) {
                                       return c.resolution == resolution;
                                   });
            it != s->caps.cend())
            return it->frameRates;
    }
    return {};
}

GstDevice *CallDevices::audioDevice() const
{
    return m_audioDevicesModel->currentDevice();
}

GstDevice *CallDevices::videoDevice(QPair<int, int> &resolution, QPair<int, int> &frameRate) const
{
    return m_videoDevicesModel->currentDevice(resolution, frameRate);
}

#else

bool CallDevices::hasMicrophone() const
{
    return false;
}

bool CallDevices::hasCamera() const
{
    return false;
}

QStringList CallDevices::names(bool, const QString &) const
{
    return {};
}

QStringList CallDevices::resolutions(const QString &) const
{
    return {};
}

QStringList CallDevices::frameRates(const QString &, const QString &) const
{
    return {};
}

#endif
