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
            auto trainName = QStringLiteral("%1 %2").arg(data[QStringLiteral("reservationFor")][QStringLiteral("trainName")].toString(),
                                                         data[QStringLiteral("reservationFor")][QStringLiteral("trainNumber")].toString());
            if (trainName.trimmed().isEmpty()) {
                return QStringLiteral("%1 to %2")
                    .arg(data[QStringLiteral("reservationFor")][QStringLiteral("departureStation")][QStringLiteral("name")].toString(),
                         data[QStringLiteral("reservationFor")][QStringLiteral("arrivalStation")][QStringLiteral("name")].toString());
                ;
            }
            return trainName;
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("LodgingReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("name")];
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("FoodEstablishmentReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("name")];
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("FlightReservation")) {
            return QStringLiteral("%1 %2 %3 → %4")
                .arg(data[QStringLiteral("reservationFor")][QStringLiteral("airline")][QStringLiteral("iataCode")].toString(),
                     data[QStringLiteral("reservationFor")][QStringLiteral("flightNumber")].toString(),
                     data[QStringLiteral("reservationFor")][QStringLiteral("departureAirport")][QStringLiteral("iataCode")].toString(),
                     data[QStringLiteral("reservationFor")][QStringLiteral("arrivalAirport")][QStringLiteral("iataCode")].toString());
        }
    }
    if (role == TypeRole) {
        return data[QStringLiteral("@type")];
    }
    if (role == DepartureLocationRole) {
        if (data[QStringLiteral("@type")] == QStringLiteral("TrainReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("departureStation")][QStringLiteral("name")];
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("FlightReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("departureAirport")][QStringLiteral("iataCode")];
        }
    }
    if (role == DepartureAddressRole) {
        if (data[QStringLiteral("@type")] == QStringLiteral("TrainReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("departureStation")][QStringLiteral("address")][QStringLiteral("addressCountry")]
                .toString();
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("FlightReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("departureAirport")][QStringLiteral("address")][QStringLiteral("addressCountry")]
                .toString();
        }
    }
    if (role == ArrivalLocationRole) {
        if (data[QStringLiteral("@type")] == QStringLiteral("TrainReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("arrivalStation")][QStringLiteral("name")];
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("FlightReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("arrivalAirport")][QStringLiteral("iataCode")];
        }
    }
    if (role == ArrivalAddressRole) {
        if (data[QStringLiteral("@type")] == QStringLiteral("TrainReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("arrivalStation")][QStringLiteral("address")][QStringLiteral("addressCountry")]
                .toString();
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("FlightReservation")) {
            return data[QStringLiteral("reservationFor")][QStringLiteral("arrivalAirport")][QStringLiteral("address")][QStringLiteral("addressCountry")]
                .toString();
        }
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
        QDateTime dateTime;
        if (data[QStringLiteral("@type")] == QStringLiteral("LodgingReservation")) {
            dateTime = data[QStringLiteral("checkinTime")][QStringLiteral("@value")].toVariant().toDateTime();
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("FoodEstablishmentReservation")) {
            dateTime = data[QStringLiteral("startTime")][QStringLiteral("@value")].toVariant().toDateTime();
        }
        if (data[QStringLiteral("@type")] == QStringLiteral("FlightReservation")) {
            dateTime = data[QStringLiteral("reservationFor")][QStringLiteral("boardingTime")][QStringLiteral("@value")].toVariant().toDateTime();
        }
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
        {DepartureLocationRole, "departureLocation"},
        {DepartureAddressRole, "departureAddress"},
        {ArrivalLocationRole, "arrivalLocation"},
        {ArrivalAddressRole, "arrivalAddress"},
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
