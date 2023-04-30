// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

#include <user.h>

/**
 * @class NeoChatUser
 *
 * A class inherited from Quotient::User to add a user color function.
 *
 * @sa Quotient::User
 */
class NeoChatUser : public Quotient::User
{
    Q_OBJECT

    /**
     * @brief The color to use for the user.
     */
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    NeoChatUser(QString userId, Quotient::Connection *connection);

public Q_SLOTS:
    QColor color();
    void setColor(const QColor &color);

Q_SIGNALS:
    void colorChanged(QColor _t1);

private:
    QColor m_color;

    void polishColor();
};
