// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "xscreencast.h"

GstElement *XScreenCast::request(int index)
{
    GstElement *bin = gst_bin_new(nullptr);
    qDebug() << "index" << index;
    GstElement *ximagesrc = gst_element_factory_make("ximagesrc", "ximagesrc");
    g_object_set(ximagesrc, "xid", index, nullptr);
    g_object_set(ximagesrc, "use-damage", true, nullptr);

    //     GstElement *filter = gst_element_factory_make("capsfilter", "capsfilter");
    //     GstCaps *caps = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "framerate", GST_TYPE_FRACTION, 30, 1,
    //     nullptr); g_object_set(filter, "caps", caps, nullptr); gst_caps_unref(caps);

    GstElement *queue = gst_element_factory_make("queue", "queue");
    gst_bin_add_many(GST_BIN(bin), ximagesrc, queue, /*filter,*/ nullptr);
    gst_element_link_many(ximagesrc, queue /*, filter*/, nullptr);

    // GstPad *pad = gst_element_get_static_pad(filter, "src");
    GstPad *pad = gst_element_get_static_pad(queue, "src");
    auto ghostpad = gst_ghost_pad_new("src", pad);
    gst_element_add_pad(bin, ghostpad);
    gst_object_unref(pad);

    return bin;
}

XScreenCast::XScreenCast(QObject *parent)
    : AbstractScreenCast(parent)
{
}

bool XScreenCast::canSelectWindow() const
{
    return true;
}

bool XScreenCast::canShareScreen() const
{
    return true;
}
