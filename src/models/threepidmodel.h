// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/csapi/administrative_contact.h>
#include <Quotient/jobs/jobhandle.h>

class NeoChatConnection;

/**
 * @class ThreePIdModel
 *
 * This class defines the model for visualising an account's 3PIDs.
 */
class ThreePIdModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current connection for the model to use.
     */
    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        AddressRole = Qt::DisplayRole, /**< The third-party identifier address. */
        MediumRole, /**< The medium of the third-party identifier. One of: [email, msisdn]. */
        IsBoundRole, /**< Whether the 3PID is bound to the current identity server. */
    };
    Q_ENUM(Roles)

    explicit ThreePIdModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatConnection *connection() const;
    void setConnection(NeoChatConnection *connection);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refreshModel();

Q_SIGNALS:
    void connectionChanged();

private:
    QPointer<NeoChatConnection> m_connection;
    QVector<Quotient::GetAccount3PIDsJob::ThirdPartyIdentifier> m_threePIds;

    Quotient::JobHandle<Quotient::GetAccount3PIDsJob> m_job;

    QList<QString> m_bindings;

    void refreshBindStatus();
};
