// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QDateTime>
#include <QQmlEngine>

/**
 * @class NeoChatDateTime
 *
 * This class is a helper for converting a QDateTime into the various format required in NeoChat.
 *
 * The intention is that this can be passed to QML and then the various Q_Properties
 * can be called to get the date/time in the desired format reading for viewing in
 * the UI.
 */
class NeoChatDateTime
{
    Q_GADGET
    QML_NAMED_ELEMENT(neoChatDateTime)

    /**
     * @brief The base QDateTime used to generate the other values.
     */
    Q_PROPERTY(QDateTime dateTime READ dateTime CONSTANT)

    /**
     * @brief The time formatted as "hh:mm".
     */
    Q_PROPERTY(QString hourMinuteString READ hourMinuteString CONSTANT)

    /**
     * @brief The date and time formatted as per QLocale::ShortFormat for your locale.
     */
    Q_PROPERTY(QString shortDateTime READ shortDateTime CONSTANT)

    /**
     * @brief The date and time formatted as per QLocale::LongFormat for your locale.
     */
    Q_PROPERTY(QString longDateTime READ longDateTime CONSTANT)

    /**
     * @brief The date formatted as relative to now.
     *
     * If the date falls within 2 days or after the current date
     * then a relative date string will be returned, such as:
     *  - Yesterday
     *  - Today
     *  - Tomorrow
     *  - Two days ago
     *  - In Two Days
     *
     * If the date falls outside this period then the format QLocale::ShortFormat
     * for your locale is used.
     */
    Q_PROPERTY(QString relativeDate READ relativeDate CONSTANT)

    /**
     * @brief The time and date formatted as relative to now.
     *
     * The format is "RelativeDate at hh::mm"
     *
     * If the date falls within 2 days before or after the current date
     * then a relative date string will be returned, such as:
     *  - Yesterday
     *  - Today
     *  - Tomorrow
     *  - Two days ago
     *  - In Two Days
     *
     * If the date falls outside this period then the format QLocale::ShortFormat
     * for your locale is used.
     */
    Q_PROPERTY(QString relativeDateTime READ relativeDateTime CONSTANT)

    /**
     * @brief The time and date formatted as relative to now.
     *
     * The format is "RelativeDate, hh::mm"
     *
     * If the date falls on the same day as current date, the date is skipped.
     * If the date falls within 2 days before current date
     * then a relative date string will be returned, such as:
     *  - Yesterday
     *  - Tomorrow
     *
     * If the date falls outside this period then the format QLocale::ShortFormat
     * for your locale is used.
     */
    Q_PROPERTY(QString shortRelativeDateTime READ shortRelativeDateTime CONSTANT)

    /**
     * @brief Whether this object has a valid date time.
     */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

public:
    NeoChatDateTime(QDateTime dateTime = {});

    QDateTime dateTime() const;

    QString hourMinuteString() const;
    QString shortDateTime() const;
    QString longDateTime() const;
    QString relativeDate() const;
    QString relativeDateTime() const;
    QString shortRelativeDateTime() const;

    bool isValid() const;

    bool operator==(const QDateTime &right) const;

private:
    QDateTime m_dateTime;
};
