// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QtCore/QAbstractListModel>

#include <gst/gst.h>

#include "devicemonitor.h"

class VideoSources : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int capsIndex READ capsIndex WRITE setCapsIndex NOTIFY capsIndexChanged)
public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        DeviceRole,
    };

    static VideoSources &instance()
    {
        static VideoSources _instance;
        return _instance;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void foo(int index);

    const VideoSource *currentDevice() const;

    void setCurrentIndex(int index);
    int currentIndex() const;

    void setCapsIndex(int index);
    int capsIndex() const;

    Q_INVOKABLE QStringList caps(int index) const;

Q_SIGNALS:
    void currentIndexChanged();
    void capsIndexChanged();

private:
    VideoSources();
};
