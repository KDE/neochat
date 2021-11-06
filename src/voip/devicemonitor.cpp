// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "devicemonitor.h"

static gboolean deviceCallback(GstBus *bus, GstMessage *message, gpointer user_data)
{
    Q_UNUSED(bus);
    auto monitor = static_cast<DeviceMonitor *>(user_data);
    return monitor->callback(message);
}

DeviceMonitor::DeviceMonitor()
    : QObject()
    , m_monitor(gst_device_monitor_new())
{
    GstBus *bus;
    GstCaps *caps;

    bus = gst_device_monitor_get_bus(m_monitor);
    gst_bus_add_watch(bus, deviceCallback, this);
    gst_object_unref(bus);

    if (!gst_device_monitor_start(m_monitor)) {
        qWarning() << "Failed to start device monitor";
    }
}

QVector<AudioSource> DeviceMonitor::audioSources() const
{
    return m_audioSources;
}

QVector<VideoSource> DeviceMonitor::videoSources() const
{
    return m_videoSources;
}

void DeviceMonitor::handleVideoSource(GstDevice *device)
{
    VideoSource source;
    source.title = QString(gst_device_get_display_name(device));
    source.device = device;

    auto caps = gst_device_get_caps(device);
    auto size = gst_caps_get_size(caps);
    for (int i = 0; i < size; i++) {
        VideoCap videoCap;
        GstStructure *cap = gst_caps_get_structure(caps, i);
        gst_structure_get(cap, "width", G_TYPE_INT, &videoCap.width, "height", G_TYPE_INT, &videoCap.height, nullptr);
        const auto framerate = gst_structure_get_value(cap, "framerate");
        if (GST_VALUE_HOLDS_FRACTION(framerate)) {
            auto numerator = gst_value_get_fraction_numerator(framerate);
            auto denominator = gst_value_get_fraction_denominator(framerate);
            videoCap.framerates += (float)numerator / denominator;
        }
        // unref cap?
        source.caps += videoCap;
    }
    m_videoSources += source;
    Q_EMIT videoSourceAdded();
}

void DeviceMonitor::handleAudioSource(GstDevice *device)
{
}

bool DeviceMonitor::callback(GstMessage *message)
{
    GstDevice *device;
    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_DEVICE_ADDED: {
        gst_message_parse_device_added(message, &device);
        auto name = gst_device_get_display_name(device);
        auto deviceClass = QString(gst_device_get_device_class(device));
        if (deviceClass == QStringLiteral("Video/Source")) {
            handleVideoSource(device);

        } else if (deviceClass == QStringLiteral("Audio/Source")) {
            AudioSource _device;
            _device.title = QString(name);
            m_audioSources += _device;
            Q_EMIT audioSourceAdded();
        }
        g_free(name);
        gst_object_unref(device);
        break;
    }
    case GST_MESSAGE_DEVICE_REMOVED: {
        gst_message_parse_device_removed(message, &device);
        auto name = gst_device_get_display_name(device);
        auto deviceClass = QString(gst_device_get_device_class(device));
        if (deviceClass == QStringLiteral("Video/Source")) {
            m_videoSources.erase(std::remove_if(m_videoSources.begin(),
                                                m_videoSources.end(),
                                                [name](VideoSource d) {
                                                    return d.title == QString(name);
                                                }),
                                 m_videoSources.end());
            Q_EMIT videoSourceRemoved();
        } else if (deviceClass == QStringLiteral("Audio/Source")) {
            m_audioSources.erase(std::remove_if(m_audioSources.begin(),
                                                m_audioSources.end(),
                                                [name](AudioSource d) {
                                                    return d.title == QString(name);
                                                }),
                                 m_audioSources.end());
            Q_EMIT audioSourceRemoved();
        }
        g_free(name);
        gst_object_unref(device);
        break;
    }
    default:
        break;
    }
    return G_SOURCE_CONTINUE;
}
