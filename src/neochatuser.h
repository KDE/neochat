/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <QObject>

#include "room.h"
#include "user.h"

using namespace Quotient;

class NeoChatUser : public User
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color CONSTANT)
    Q_PROPERTY(QString defaultName READ defaultName WRITE setDefaultName NOTIFY nameChanged)
public:
    NeoChatUser(QString userId, Connection *connection)
        : User(userId, connection)
    {
    }

    QColor color();

    // TODO libQuotient 0.7: remove
    void setDefaultName(QString defaultName);
    QString defaultName();

Q_SIGNALS:
    void nameChanged();

private:
    QString m_defaultName;
};
