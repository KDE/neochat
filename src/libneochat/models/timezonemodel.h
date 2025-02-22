// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

class TimeZoneModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TimeZoneModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    Q_INVOKABLE int indexOfValue(const QString &code);

private:
    QList<QByteArray> m_timezoneIds;
};
