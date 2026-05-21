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
    explicit ItineraryModel(const QUrl &source, QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    QHash<int, QByteArray> roleNames() const override;

    bool loading() const;

    Q_INVOKABLE void sendToItinerary();

Q_SIGNALS:
    void loaded();
    void loadErrorOccurred();

private:
    QJsonArray m_data;
    QUrl m_source;
    void loadData();
    bool m_loading = false;
};
