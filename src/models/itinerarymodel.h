// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QPointer>
#include <QQmlEngine>
#include <QString>

#include "neochatconnection.h"

class ItineraryModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

public:
    enum Roles {
        NameRole = Qt::DisplayRole,
        TypeRole,
        DepartureStationRole,
        ArrivalStationRole,
        DepartureTimeRole,
        ArrivalTimeRole,
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

    void setConnection(NeoChatConnection *connection);
    NeoChatConnection *connection() const;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    QHash<int, QByteArray> roleNames() const override;

    QString path() const;
    void setPath(const QString &path);

    Q_INVOKABLE void sendToItinerary();

Q_SIGNALS:
    void connectionChanged();
    void pathChanged();

private:
    QPointer<NeoChatConnection> m_connection;
    QJsonArray m_data;
    QString m_path;
    void loadData();
};
