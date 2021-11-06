// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "videosources.h"

#include <gst/gst.h>

#include "pipelinemanager.h"
#include <QDebug>
#include <QString>

#include "devicemonitor.h"

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
        return DeviceMonitor::instance().videoSources()[index.row()].title;
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
    });
    connect(&DeviceMonitor::instance(), &DeviceMonitor::videoSourceRemoved, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

void VideoSources::foo(int index)
{
    auto device = DeviceMonitor::instance().videoSources()[index].device;

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
    PipelineManager::instance().add(bin);
}
