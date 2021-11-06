// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "audiosources.h"

#include <gst/gst.h>

#include <QDebug>
#include <QString>

#include "devicemonitor.h"

int AudioSources::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return DeviceMonitor::instance().audioSources().size();
}

QVariant AudioSources::data(const QModelIndex &index, int role) const
{
    if (index.row() >= DeviceMonitor::instance().audioSources().size()) {
        return QVariant(QStringLiteral("DEADBEEF"));
    }
    if (role == TitleRole) {
        return DeviceMonitor::instance().audioSources()[index.row()].title;
    }
    return QVariant();
}

QHash<int, QByteArray> AudioSources::roleNames() const
{
    return {
        {TitleRole, "title"},
    };
}

AudioSources::AudioSources()
    : QAbstractListModel()
{
    connect(&DeviceMonitor::instance(), &DeviceMonitor::audioSourceAdded, this, [this]() {
        beginResetModel();
        endResetModel();
    });
    connect(&DeviceMonitor::instance(), &DeviceMonitor::audioSourceRemoved, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}
