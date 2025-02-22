// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>

#include <Quotient/jobs/basejob.h>

class NeoChatConnection;

/**
 * @class ProfileFieldsHelper
 *
 * This class is designed to help grabbing the profile fields of a user.
 */
class ProfileFieldsHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The connection to use.
     */
    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged REQUIRED)

    /**
     * @brief The id of the user to grab profile fields from.
     */
    Q_PROPERTY(QString userId READ userId WRITE setUserId NOTIFY userIdChanged REQUIRED)

    /**
     * @brief The timezone field of the user.
     */
    Q_PROPERTY(QString timezone READ timezone NOTIFY timezoneChanged)

    /**
     * @brief The pronouns field of the user.
     */
    Q_PROPERTY(QString pronouns READ pronouns NOTIFY pronounsChanged)

    /**
     * @brief If the fields are loading.
     */
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    [[nodiscard]] NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

    [[nodiscard]] QString userId() const;
    void setUserId(const QString &id);

    [[nodiscard]] QString timezone() const;
    [[nodiscard]] QString pronouns() const;

    [[nodiscard]] bool loading() const;

Q_SIGNALS:
    void connectionChanged();
    void userIdChanged();
    void timezoneChanged();
    void pronounsChanged();
    void loadingChanged();

private:
    void load();
    void checkIfFinished();
    void setLoading(bool loading);

    QPointer<NeoChatConnection> m_connection;
    QString m_userId;
    bool m_loading = true;
    QString m_timezone;
    bool m_fetchedTimezone = false;
    QString m_pronouns;
    bool m_fetchedPronouns = false;
};
