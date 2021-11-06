// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pipelinemanager.h"
#include <QtCore/QDebug>
#include <QtCore/QThread>

Q_DECLARE_METATYPE(GstElement *);

PipelineManager::PipelineManager()
{
}

GstElement *SinkModel::get(int index) const
{
    return m_bins[index];
}

void PipelineManager::show(QQuickItem *item, int index)
{
    auto pipeline = gst_pipeline_new(nullptr);

    auto bin = SinkModel::instance().get(index);
    if (gst_object_get_parent(GST_OBJECT(bin))) {
        return;
    }

    GstElement *glUpload = gst_element_factory_make("glupload", nullptr);
    GstElement *glcolorconvert = gst_element_factory_make("glcolorconvert", nullptr);
    GstElement *glcolorbalance = gst_element_factory_make("glcolorbalance", nullptr);
    GstElement *glImageSink = gst_element_factory_make("qmlglsink", nullptr);
    g_object_set(glImageSink, "widget", item, nullptr);

    gst_bin_add_many(GST_BIN(pipeline), bin, glUpload, glcolorconvert, glcolorbalance, glImageSink, nullptr);
    gst_element_link_many(bin, glUpload, glcolorconvert, glcolorbalance, glImageSink, nullptr);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void PipelineManager::add(GstElement *bin)
{
    SinkModel::instance().add(bin);
}
void SinkModel::add(GstElement *bin)
{
    beginInsertRows(QModelIndex(), m_bins.size(), m_bins.size());
    m_bins += bin;
    endInsertRows();
}

QHash<int, QByteArray> SinkModel::roleNames() const
{
    return {{Element, "element"}};
}

int SinkModel::rowCount(const QModelIndex &parent) const
{
    return m_bins.size();
}
QVariant SinkModel::data(const QModelIndex &index, int role) const
{
    return QVariant::fromValue(m_bins[index.row()]);
}
