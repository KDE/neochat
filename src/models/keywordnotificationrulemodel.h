// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <csapi/definitions/push_rule.h>

#include <QAbstractListModel>

class KeywordNotificationRuleModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum EventRoles {
        NameRole = Qt::DisplayRole,
    };

    KeywordNotificationRuleModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addKeyword(const QString &keyword);
    Q_INVOKABLE void removeKeywordAtIndex(int index);

private Q_SLOTS:
    void controllerConnectionChanged();
    void updateNotificationRules(const QString &type);

private:
    QList<QString> m_notificationRules;
};
