// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <csapi/definitions/push_rule.h>

#include <QAbstractListModel>

/**
 * @class KeywordNotificationRuleModel
 *
 * This class defines the model for managing notification push rule keywords.
 */
class KeywordNotificationRuleModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * @brief Defines the model roles.
     */
    enum EventRoles {
        NameRole = Qt::DisplayRole, /**< The push rule keyword. */
    };

    KeywordNotificationRuleModel(QObject *parent = nullptr);

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

    /**
     * @brief Add a new keyword to the model.
     */
    Q_INVOKABLE void addKeyword(const QString &keyword);

    /**
     * @brief Remove a keyword from the model.
     */
    Q_INVOKABLE void removeKeywordAtIndex(int index);

private Q_SLOTS:
    void controllerConnectionChanged();
    void updateNotificationRules(const QString &type);

private:
    QList<QString> m_notificationRules;
};
