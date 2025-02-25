// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QJsonArray>
#include <QPointer>
#include <QQmlEngine>
#include <QString>

class ItineraryModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    enum Roles {
        NameRole = Qt::DisplayRole,
        TypeRole,
        DepartureLocationRole,
        ArrivalLocationRole,
        DepartureTimeRole,
        DepartureAddressRole,
        ArrivalTimeRole,
        ArrivalAddressRole,
        AddressRole,
        StartTimeRole,
        EndTimeRole,
        DeparturePlatformRole,
        ArrivalPlatformRole,
        CoachRole,
        SeatRole,
    };
    Q_ENUM(Roles)
    explicit ItineraryModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    QHash<int, QByteArray> roleNames() const override;

    QString path() const;
    void setPath(const QString &path);

    Q_INVOKABLE void sendToItinerary();

Q_SIGNALS:
    void loaded();
    void loadErrorOccurred();

private:
    QJsonArray m_data;
    QString m_path;
    void loadData();
};
