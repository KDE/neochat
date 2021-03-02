// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "videosources.h"

#include <gst/gst.h>

// #include "pipelinemanager.h"
#include <QDebug>
#include <QString>

#include "devicemonitor.h"
#include "neochatconfig.h"

int VideoSources::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return DeviceMonitor::instance().videoSources().size();
}

QVariant VideoSources::data(const QModelIndex &index, int role) const
{
    if (index.row() >= DeviceMonitor::instance().videoSources().size()) {
        return QVariant(QStringLiteral("DEADBEEF"));
    }
    if (role == TitleRole) {
        return DeviceMonitor::instance().videoSources()[index.row()]->title;
    }
    return QVariant();
}

QHash<int, QByteArray> VideoSources::roleNames() const
{
    return {
        {TitleRole, "title"},
    };
}

VideoSources::VideoSources()
    : QAbstractListModel()
{
    connect(&DeviceMonitor::instance(), &DeviceMonitor::videoSourceAdded, this, [this]() {
        beginResetModel();
        endResetModel();
        Q_EMIT currentIndexChanged();
    });
    connect(&DeviceMonitor::instance(), &DeviceMonitor::videoSourceRemoved, this, [this]() {
        beginResetModel();
        endResetModel();
        Q_EMIT currentIndexChanged();
    });
}

void VideoSources::foo(int index)
{
    auto device = DeviceMonitor::instance().videoSources()[index]->device;

    auto bin = gst_bin_new(nullptr);

    GstElement *videoconvert = gst_element_factory_make("videoconvert", nullptr);
    // GstElement *videorate = gst_element_factory_make("videorate", nullptr);

    GstElement *filter = gst_element_factory_make("capsfilter", nullptr);
    GstCaps *caps = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "framerate", GST_TYPE_FRACTION, 5, 1, nullptr);
    g_object_set(filter, "caps", caps, nullptr);
    gst_caps_unref(caps);
    GstElement *deviceElement = gst_device_create_element(device, nullptr);

    gst_bin_add_many(GST_BIN(bin), deviceElement, videoconvert, filter, nullptr);
    gst_element_link_many(deviceElement, videoconvert, filter, nullptr);

    // GstPad *pad = gst_element_get_static_pad(filter, "src");
    GstPad *pad = gst_element_get_static_pad(filter, "src");
    auto ghostpad = gst_ghost_pad_new("src", pad);
    gst_element_add_pad(bin, ghostpad);
    gst_object_unref(pad);
    // PipelineManager::instance().add(bin);
}

const VideoSource *VideoSources::currentDevice() const
{
    const auto config = NeoChatConfig::self();
    const QString name = config->camera();
    for (const auto &videoSource : DeviceMonitor::instance().videoSources()) {
        if (videoSource->title == name) {
            qDebug() << "WebRTC: camera:" << name;
            return videoSource;
        }
    }
    if (DeviceMonitor::instance().videoSources().length() == 0) {
        return nullptr;
    }
    return DeviceMonitor::instance().videoSources()[0];
}

void VideoSources::setCurrentIndex(int index)
{
    if (DeviceMonitor::instance().videoSources().size() == 0) {
        return;
    }
    NeoChatConfig::setCamera(DeviceMonitor::instance().videoSources()[index]->title);
    NeoChatConfig::self()->save();

    setCapsIndex(0);
}

int VideoSources::currentIndex() const
{
    const auto config = NeoChatConfig::self();
    const QString name = config->camera();
    for (auto i = 0; i < DeviceMonitor::instance().videoSources().size(); i++) {
        if (DeviceMonitor::instance().videoSources()[i]->title == name) {
            return i;
        }
    }
    return 0;
}

QStringList VideoSources::caps(int index) const
{
    if (index >= DeviceMonitor::instance().videoSources().size()) {
        return QStringList();
    }
    const auto &caps = DeviceMonitor::instance().videoSources()[index]->caps;
    QStringList strings;
    for (const auto &cap : caps) {
        strings += QStringLiteral("%1x%2, %3 FPS").arg(cap.width).arg(cap.height).arg(cap.framerates.back());
    }
    return strings;
}

void VideoSources::setCapsIndex(int index)
{
    NeoChatConfig::self()->setCameraCaps(index);
    NeoChatConfig::self()->save();
    Q_EMIT capsIndexChanged();
}

int VideoSources::capsIndex() const
{
    return NeoChatConfig::self()->cameraCaps();
}
