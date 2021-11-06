// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QtCore/QAbstractListModel>
#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtQuick/QQuickItem>
#include <gst/gst.h>

class PipelineManager : public QObject
{
    Q_OBJECT
public:
    static PipelineManager &instance()
    {
        static PipelineManager _instance;
        return _instance;
    }

    Q_INVOKABLE void show(QQuickItem *item, int index);

    void add(GstElement *bin);

private:
    PipelineManager();
};

class SinkModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        Element = Qt::UserRole + 1,
    };

    static SinkModel &instance()
    {
        static SinkModel _instance;
        return _instance;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void add(GstElement *bin);
    GstElement *get(int index) const;

private:
    QVector<GstElement *> m_bins;
};
