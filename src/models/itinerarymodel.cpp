// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "itinerarymodel.h"

#include <QJsonDocument>
#include <QProcess>

#include "config-neochat.h"

#ifndef Q_OS_ANDROID
#include <KIO/ApplicationLauncherJob>
#endif

using namespace Qt::StringLiterals;

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
        if (data["@type"_L1] == u"TrainReservation"_s) {
            auto trainName = u"%1 %2"_s.arg(data["reservationFor"_L1]["trainName"_L1].toString(), data["reservationFor"_L1]["trainNumber"_L1].toString());
            if (trainName.trimmed().isEmpty()) {
                return u"%1 to %2"_s.arg(data["reservationFor"_L1]["departureStation"_L1]["name"_L1].toString(),
                                         data["reservationFor"_L1]["arrivalStation"_L1]["name"_L1].toString());
                ;
            }
            return trainName;
        }
        if (data["@type"_L1] == u"LodgingReservation"_s) {
            return data["reservationFor"_L1]["name"_L1];
        }
        if (data["@type"_L1] == u"FoodEstablishmentReservation"_s) {
            return data["reservationFor"_L1]["name"_L1];
        }
        if (data["@type"_L1] == u"FlightReservation"_s) {
            return u"%1 %2 %3 → %4"_s.arg(data["reservationFor"_L1]["airline"_L1]["iataCode"_L1].toString(),
                                          data["reservationFor"_L1]["flightNumber"_L1].toString(),
                                          data["reservationFor"_L1]["departureAirport"_L1]["iataCode"_L1].toString(),
                                          data["reservationFor"_L1]["arrivalAirport"_L1]["iataCode"_L1].toString());
        }
    }
    if (role == TypeRole) {
        return data["@type"_L1];
    }
    if (role == DepartureLocationRole) {
        if (data["@type"_L1] == u"TrainReservation"_s) {
            return data["reservationFor"_L1]["departureStation"_L1]["name"_L1];
        }
        if (data["@type"_L1] == u"FlightReservation"_s) {
            return data["reservationFor"_L1]["departureAirport"_L1]["iataCode"_L1];
        }
    }
    if (role == DepartureAddressRole) {
        if (data["@type"_L1] == u"TrainReservation"_s) {
            return data["reservationFor"_L1]["departureStation"_L1]["address"_L1]["addressCountry"_L1].toString();
        }
        if (data["@type"_L1] == u"FlightReservation"_s) {
            return data["reservationFor"_L1]["departureAirport"_L1]["address"_L1]["addressCountry"_L1].toString();
        }
    }
    if (role == ArrivalLocationRole) {
        if (data["@type"_L1] == u"TrainReservation"_s) {
            return data["reservationFor"_L1]["arrivalStation"_L1]["name"_L1];
        }
        if (data["@type"_L1] == u"FlightReservation"_s) {
            return data["reservationFor"_L1]["arrivalAirport"_L1]["iataCode"_L1];
        }
    }
    if (role == ArrivalAddressRole) {
        if (data["@type"_L1] == u"TrainReservation"_s) {
            return data["reservationFor"_L1]["arrivalStation"_L1]["address"_L1]["addressCountry"_L1].toString();
        }
        if (data["@type"_L1] == u"FlightReservation"_s) {
            return data["reservationFor"_L1]["arrivalAirport"_L1]["address"_L1]["addressCountry"_L1].toString();
        }
    }
    if (role == DepartureTimeRole) {
        const auto &time = data["reservationFor"_L1]["departureTime"_L1];
        auto dateTime = (time.isString() ? time : time["@value"_L1]).toVariant().toDateTime();
        if (const auto &timeZone = time["timezone"_L1].toString(); timeZone.length() > 0) {
            dateTime.setTimeZone(QTimeZone(timeZone.toLatin1().data()));
        }
        return dateTime.toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
    }
    if (role == ArrivalTimeRole) {
        const auto &time = data["reservationFor"_L1]["arrivalTime"_L1];
        auto dateTime = (time.isString() ? time : time["@value"_L1]).toVariant().toDateTime();
        if (const auto &timeZone = time["timezone"_L1].toString(); timeZone.length() > 0) {
            dateTime.setTimeZone(QTimeZone(timeZone.toLatin1().data()));
        }
        return dateTime.toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
    }
    if (role == AddressRole) {
        const auto &addressData = data["reservationFor"_L1]["address"_L1];
        return u"%1 - %2 %3 %4"_s.arg(addressData["streetAddress"_L1].toString(),
                                      addressData["postalCode"_L1].toString(),
                                      addressData["addressLocality"_L1].toString(),
                                      addressData["addressCountry"_L1].toString());
    }
    if (role == StartTimeRole) {
        QDateTime dateTime;
        if (data["@type"_L1] == u"LodgingReservation"_s) {
            dateTime = data["checkinTime"_L1]["@value"_L1].toVariant().toDateTime();
        }
        if (data["@type"_L1] == u"FoodEstablishmentReservation"_s) {
            dateTime = data["startTime"_L1]["@value"_L1].toVariant().toDateTime();
        }
        if (data["@type"_L1] == u"FlightReservation"_s) {
            dateTime = data["reservationFor"_L1]["boardingTime"_L1]["@value"_L1].toVariant().toDateTime();
        }
        return dateTime.toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
    }
    if (role == EndTimeRole) {
        auto dateTime = data["checkoutTime"_L1]["@value"_L1].toVariant().toDateTime();
        return dateTime.toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
    }
    if (role == DeparturePlatformRole) {
        return data["reservationFor"_L1]["departurePlatform"_L1];
    }
    if (role == ArrivalPlatformRole) {
        return data["reservationFor"_L1]["arrivalPlatform"_L1];
    }
    if (role == CoachRole) {
        return data["reservedTicket"_L1]["ticketedSeat"_L1]["seatSection"_L1];
    }
    if (role == SeatRole) {
        return data["reservedTicket"_L1]["ticketedSeat"_L1]["seatNumber"_L1];
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
    process->start(QStringLiteral(CMAKE_INSTALL_FULL_LIBEXECDIR_KF6) + u"/kitinerary-extractor"_s, {m_path.mid(7)});
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
    auto job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(u"org.kde.itinerary"_s));
    job->setUrls({QUrl::fromLocalFile(m_path.mid(7))});
    job->start();
#endif
}

#include "moc_itinerarymodel.cpp"
