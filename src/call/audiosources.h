// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QtCore/QAbstractListModel>

#include <gst/gst.h>

class AudioSources : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
    };

    static AudioSources &instance()
    {
        static AudioSources _instance;
        return _instance;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    GstDevice *currentDevice() const;

    void setCurrentIndex(int index) const;
    int currentIndex() const;

Q_SIGNALS:
    void currentIndexChanged();

private:
    AudioSources();
    int getDefaultDeviceIndex() const;
};
