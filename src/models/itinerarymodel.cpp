// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "itinerarymodel.h"

#include <QJsonDocument>
#include <QProcess>

#include "config-neochat.h"

#ifndef Q_OS_ANDROID
#include <KIO/ApplicationLauncherJob>
#endif

ItineraryModel::ItineraryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant ItineraryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    auto row = index.row();
    auto data = m_data[row];
    if (role == NameRole) {
        if (data[QStringLiteral("@type")] == QStringLiteral("TrainReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("trainNumber")];
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("LodgingReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("name")];
        }
    }
    if (role == TypeRole) {
        return data[QStringLiteral("@type")];
    }
    if (role == DepartureStationRole) {
        return data[QStringLiteral("reservationFor")][QStringLiteral("departureStation")][QStringLiteral("name")];
    }
    if (role == ArrivalStationRole) {
        return data[QStringLiteral("reservationFor")][QStringLiteral("arrivalStation")][QStringLiteral("name")];
    }
    if (role == DepartureTimeRole) {
        const auto &time = data[QStringLiteral("reservationFor")][QStringLiteral("departureTime")];
        auto dateTime = (time.isString() ? time : time[QStringLiteral("@value")]).toVariant().toDateTime();
        if (const auto &timeZone = time[QStringLiteral("timezone")].toString(); timeZone.length() > 0) {
            dateTime.setTimeZone(QTimeZone(timeZone.toLatin1().data()));
        }
        return dateTime.toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
    }
    if (role == ArrivalTimeRole) {
        const auto &time = data[QStringLiteral("reservationFor")][QStringLiteral("arrivalTime")];
        auto dateTime = (time.isString() ? time : time[QStringLiteral("@value")]).toVariant().toDateTime();
        if (const auto &timeZone = time[QStringLiteral("timezone")].toString(); timeZone.length() > 0) {
            dateTime.setTimeZone(QTimeZone(timeZone.toLatin1().data()));
        }
        return dateTime.toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
    }
    if (role == AddressRole) {
        const auto &addressData = data[QStringLiteral("reservationFor")][QStringLiteral("address")];
        return QStringLiteral("%1 - %2 %3 %4")
            .arg(addressData[QStringLiteral("streetAddress")].toString(),
                 addressData[QStringLiteral("postalCode")].toString(),
                 addressData[QStringLiteral("addressLocality")].toString(),
                 addressData[QStringLiteral("addressCountry")].toString());
    }
    if (role == StartTimeRole) {
        auto dateTime = data[QStringLiteral("checkinTime")][QStringLiteral("@value")].toVariant().toDateTime();
        return dateTime.toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
    }
    if (role == EndTimeRole) {
        auto dateTime = data[QStringLiteral("checkoutTime")][QStringLiteral("@value")].toVariant().toDateTime();
        return dateTime.toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
    }
    if (role == DeparturePlatformRole) {
        return data[QStringLiteral("reservationFor")][QStringLiteral("departurePlatform")];
    }
    if (role == ArrivalPlatformRole) {
        return data[QStringLiteral("reservationFor")][QStringLiteral("arrivalPlatform")];
    }
    if (role == CoachRole) {
        return data[QStringLiteral("reservedTicket")][QStringLiteral("ticketedSeat")][QStringLiteral("seatSection")];
    }
    if (role == SeatRole) {
        return data[QStringLiteral("reservedTicket")][QStringLiteral("ticketedSeat")][QStringLiteral("seatNumber")];
    }
    return {};
}

int ItineraryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

QHash<int, QByteArray> ItineraryModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {TypeRole, "type"},
        {DepartureStationRole, "departureStation"},
        {ArrivalStationRole, "arrivalStation"},
        {DepartureTimeRole, "departureTime"},
        {ArrivalTimeRole, "arrivalTime"},
        {AddressRole, "address"},
        {StartTimeRole, "startTime"},
        {EndTimeRole, "endTime"},
        {DeparturePlatformRole, "departurePlatform"},
        {ArrivalPlatformRole, "arrivalPlatform"},
        {CoachRole, "coach"},
        {SeatRole, "seat"},
    };
}

QString ItineraryModel::path() const
{
    return m_path;
}

void ItineraryModel::setPath(const QString &path)
{
    m_path = path;
    loadData();
}

void ItineraryModel::loadData()
{
    auto process = new QProcess(this);
    process->start(QLatin1String(CMAKE_INSTALL_FULL_LIBEXECDIR_KF6) + QLatin1String("/kitinerary-extractor"), {m_path.mid(7)});
    connect(process, &QProcess::finished, this, [this, process]() {
        auto data = process->readAllStandardOutput();
        beginResetModel();
        m_data = QJsonDocument::fromJson(data).array();
        endResetModel();

        Q_EMIT loaded();
    });
    connect(process, &QProcess::errorOccurred, this, [this]() {
        Q_EMIT loadErrorOccurred();
    });
}

void ItineraryModel::sendToItinerary()
{
#ifndef Q_OS_ANDROID
    auto job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(QStringLiteral("org.kde.itinerary")));
    job->setUrls({QUrl::fromLocalFile(m_path.mid(7))});
    job->start();
#endif
}
