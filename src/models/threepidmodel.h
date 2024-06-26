// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/csapi/administrative_contact.h>

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
    QML_UNCREATABLE("")

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        AddressRole = Qt::DisplayRole, /**< The third-party identifier address. */
        MediumRole, /**< The medium of the third-party identifier. One of: [email, msisdn]. */
        IsBoundRole, /**< Whether the 3PID is bound to the current identity server. */
    };

    explicit ThreePIdModel(NeoChatConnection *parent);

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

private:
    QVector<Quotient::GetAccount3PIDsJob::ThirdPartyIdentifier> m_threePIds;

    QList<QString> m_bindings;

    void refreshBindStatus();
};
