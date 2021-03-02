// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "audiosources.h"

#include <gst/gst.h>

#include <QDebug>
#include <QString>

#include "devicemonitor.h"

#include "neochatconfig.h"

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
        return DeviceMonitor::instance().audioSources()[index.row()]->title;
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
        Q_EMIT currentIndexChanged();
    });
    connect(&DeviceMonitor::instance(), &DeviceMonitor::audioSourceRemoved, this, [this]() {
        beginResetModel();
        endResetModel();
        Q_EMIT currentIndexChanged();
    });
}

GstDevice *AudioSources::currentDevice() const
{
    const auto config = NeoChatConfig::self();
    const QString name = config->microphone();
    for (const auto &audioSource : DeviceMonitor::instance().audioSources()) {
        if (audioSource->title == name) {
            qDebug() << "WebRTC: microphone:" << name;
            return audioSource->device;
        }
    }
    return DeviceMonitor::instance().audioSources()[0]->device;
}

void AudioSources::setCurrentIndex(int index) const
{
    if (DeviceMonitor::instance().audioSources().size() == 0) {
        return;
    }
    NeoChatConfig::setMicrophone(DeviceMonitor::instance().audioSources()[index]->title);
    NeoChatConfig::self()->save();
}

int AudioSources::currentIndex() const
{
    const auto config = NeoChatConfig::self();
    const QString name = config->microphone();
    if (name.isEmpty()) {
        return getDefaultDeviceIndex();
    }
    for (auto i = 0; i < DeviceMonitor::instance().audioSources().size(); i++) {
        if (DeviceMonitor::instance().audioSources()[i]->title == name) {
            return i;
        }
    }
    return 0;
}

int AudioSources::getDefaultDeviceIndex() const
{
    for (auto i = 0; i < DeviceMonitor::instance().audioSources().size(); i++) {
        if (DeviceMonitor::instance().audioSources()[i]->isDefault) {
            return i;
        }
    }
    return 0;
}
