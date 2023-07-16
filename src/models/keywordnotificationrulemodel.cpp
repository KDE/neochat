// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "keywordnotificationrulemodel.h"
#include "controller.h"
#include "notificationsmanager.h"

#include <QDebug>
#include <Quotient/connection.h>
#include <Quotient/converters.h>
#include <Quotient/csapi/definitions/push_ruleset.h>
#include <Quotient/csapi/pushrules.h>
#include <Quotient/jobs/basejob.h>

KeywordNotificationRuleModel::KeywordNotificationRuleModel(QObject *parent)
    : QAbstractListModel(parent)
{
    if (Controller::instance().activeConnection()) {
        controllerConnectionChanged();
    }
    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, &KeywordNotificationRuleModel::controllerConnectionChanged);
}

void KeywordNotificationRuleModel::controllerConnectionChanged()
{
    connect(Controller::instance().activeConnection(), &Quotient::Connection::accountDataChanged, this, &KeywordNotificationRuleModel::updateNotificationRules);
    updateNotificationRules("m.push_rules");
}

void KeywordNotificationRuleModel::updateNotificationRules(const QString &type)
{
    if (type != "m.push_rules") {
        return;
    }

    const QJsonObject ruleDataJson = Controller::instance().activeConnection()->accountDataJson("m.push_rules");
    const Quotient::PushRuleset ruleData = Quotient::fromJson<Quotient::PushRuleset>(ruleDataJson["global"].toObject());
    const QVector<Quotient::PushRule> contentRules = ruleData.content;

    beginResetModel();
    m_notificationRules.clear();
    for (const auto &i : contentRules) {
        if (!m_notificationRules.contains(i.ruleId) && i.ruleId[0] != '.') {
            m_notificationRules.append(i.ruleId);
        }
    }
    endResetModel();
}

QVariant KeywordNotificationRuleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= m_notificationRules.count()) {
        qDebug() << "KeywordNotificationRuleModel, something's wrong: index.row() >= m_notificationRules.count()";
        return {};
    }

    if (role == NameRole) {
        return m_notificationRules.at(index.row());
    }

    return {};
}

int KeywordNotificationRuleModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_notificationRules.count();
}

void KeywordNotificationRuleModel::addKeyword(const QString &keyword)
{
    if (m_notificationRules.count() == 0) {
        NotificationsManager::instance().initializeKeywordNotificationAction();
    }

    const QVector<QVariant> actions = NotificationsManager::instance().getKeywordNotificationActions();

    auto job = Controller::instance()
                   .activeConnection()
                   ->callApi<Quotient::SetPushRuleJob>("global", "content", keyword, actions, "", "", QVector<Quotient::PushCondition>(), keyword);
    connect(job, &Quotient::BaseJob::success, this, [this, keyword]() {
        beginInsertRows(QModelIndex(), m_notificationRules.count(), m_notificationRules.count());
        m_notificationRules.append(keyword);
        endInsertRows();
    });
}

void KeywordNotificationRuleModel::removeKeywordAtIndex(int index)
{
    auto job = Controller::instance().activeConnection()->callApi<Quotient::DeletePushRuleJob>("global", "content", m_notificationRules[index]);
    connect(job, &Quotient::BaseJob::success, this, [this, index]() {
        beginRemoveRows(QModelIndex(), index, index);
        m_notificationRules.removeAt(index);
        endRemoveRows();

        if (m_notificationRules.count() == 0) {
            NotificationsManager::instance().deactivateKeywordNotificationAction();
        }
    });
}

QHash<int, QByteArray> KeywordNotificationRuleModel::roleNames() const
{
    return {{NameRole, QByteArrayLiteral("name")}};
}

#include "moc_keywordnotificationrulemodel.cpp"
