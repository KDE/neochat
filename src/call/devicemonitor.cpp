// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "devicemonitor.h"
#include "voiplogging.h"
#include <QTimer>

QDebug operator<<(QDebug dbg, const GstStructure *props)
{
    QDebugStateSaver saver(dbg);
    auto asStr = gst_structure_to_string(props);
    dbg << asStr;
    g_free(asStr);
    return dbg;
}

static gboolean deviceCallback(GstBus *bus, GstMessage *message, gpointer user_data)
{
    Q_UNUSED(bus);
    auto monitor = static_cast<DeviceMonitor *>(user_data);
    return monitor->callback(message);
}

DeviceMonitor::DeviceMonitor()
    : QObject()
{
    QTimer::singleShot(0, this, &DeviceMonitor::init);
}

void DeviceMonitor::init()
{
    if (m_monitor) {
        return;
    }
    m_monitor = gst_device_monitor_new();
    GstCaps *caps = gst_caps_new_empty_simple("audio/x-raw");
    gst_device_monitor_add_filter(m_monitor, "Audio/Source", caps);

    gst_caps_unref(caps);
    caps = gst_caps_new_empty_simple("video/x-raw");
    gst_device_monitor_add_filter(m_monitor, "Video/Source", caps);
    gst_caps_unref(caps);

    GstBus *bus = gst_device_monitor_get_bus(m_monitor);
    gst_bus_add_watch(bus, deviceCallback, this);
    gst_object_unref(bus);

    if (!gst_device_monitor_start(m_monitor)) {
        qWarning() << "Failed to start device monitor";
    }
}

QVector<AudioSource *> DeviceMonitor::audioSources() const
{
    return m_audioSources;
}

QVector<VideoSource *> DeviceMonitor::videoSources() const
{
    return m_videoSources;
}

void DeviceMonitor::handleVideoSource(GstDevice *device)
{
    auto source = new VideoSource();
    auto title = gst_device_get_display_name(device);
    source->title = QString(title);
    g_free(title);
    source->device = device;

    auto caps = gst_device_get_caps(device);
    auto size = gst_caps_get_size(caps);
    for (size_t i = 0; i < size; i++) {
        VideoCap videoCap;
        GstStructure *cap = gst_caps_get_structure(caps, i);
        const gchar *name = gst_structure_get_name(cap);
        if (strcmp(name, "video/x-raw")) {
            // TODO g_free(name);
            continue;
        }
        // TODO g_free(name);
        gst_structure_get(cap, "width", G_TYPE_INT, &videoCap.width, "height", G_TYPE_INT, &videoCap.height, nullptr);
        const auto framerate = gst_structure_get_value(cap, "framerate");
        if (GST_VALUE_HOLDS_FRACTION(framerate)) {
            auto numerator = gst_value_get_fraction_numerator(framerate);
            auto denominator = gst_value_get_fraction_denominator(framerate);
            videoCap.framerates += (float)numerator / denominator;
        }
        // unref cap?
        source->caps += videoCap;
    }
    m_videoSources += source;
    Q_EMIT videoSourceAdded();
}

void DeviceMonitor::handleAudioSource(GstDevice *device)
{
    auto source = new AudioSource();
    auto title = gst_device_get_display_name(device);
    source->title = QString(title);
    g_free(title);

    GstStructure *props = gst_device_get_properties(device);
    gboolean isDefault = false;
    if (gst_structure_has_field(props, "is-default")) {
        gst_structure_get_boolean(props, "is-default", &isDefault);
    }
    gst_structure_free(props);
    source->isDefault = isDefault;

    source->device = device;
    m_audioSources += source;
    Q_EMIT audioSourceAdded();
}

bool DeviceMonitor::callback(GstMessage *message)
{
    GstDevice *device;
    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_DEVICE_ADDED: {
        gst_message_parse_device_added(message, &device);
        auto name = gst_device_get_display_name(device);
        auto props = gst_device_get_properties(device);
        qCDebug(voip) << name << props;
        gst_structure_free(props);
        if (gst_device_has_classes(device, "Video/Source")) {
            handleVideoSource(device);
        } else if (gst_device_has_classes(device, "Audio/Source")) {
            handleAudioSource(device);
        }
        g_free(name);
        gst_object_unref(device);
        break;
    }
    case GST_MESSAGE_DEVICE_REMOVED: {
        gst_message_parse_device_removed(message, &device);
        auto name = gst_device_get_display_name(device);
        auto props = gst_device_get_properties(device);
        qCDebug(voip) << name << props;
        if (gst_device_has_classes(device, "Video/Source")) {
            m_videoSources.erase(std::remove_if(m_videoSources.begin(),
                                                m_videoSources.end(),
                                                [name](auto d) {
                                                    return d->title == QString(name);
                                                }),
                                 m_videoSources.end());
            Q_EMIT videoSourceRemoved();
        } else if (gst_device_has_classes(device, "Audio/Source")) {
            m_audioSources.erase(std::remove_if(m_audioSources.begin(),
                                                m_audioSources.end(),
                                                [name](auto d) {
                                                    return d->title == QString(name);
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
