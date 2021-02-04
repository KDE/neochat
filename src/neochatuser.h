/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <QObject>

#include "room.h"
#include "user.h"

namespace Kirigami
{
class PlatformTheme;
}

using namespace Quotient;

class NeoChatUser : public User
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    NeoChatUser(QString userId, Connection *connection);

public Q_SLOTS:
    QColor color();
    void setColor(const QColor &color);

Q_SIGNALS:
    void colorChanged(QColor _t1);

private:
    QColor m_color;

    void polishColor();
};
