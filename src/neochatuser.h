// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

#include <user.h>

class NeoChatUser : public Quotient::User
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    // Only valid for the local user
    Q_PROPERTY(QString accountLabel READ accountLabel WRITE setAccountLabel NOTIFY accountLabelChanged)
public:
    NeoChatUser(QString userId, Quotient::Connection *connection);

public Q_SLOTS:
    QColor color();
    void setColor(const QColor &color);

    // Only valid for the local user
    QString accountLabel() const;
    void setAccountLabel(const QString &accountLabel);

Q_SIGNALS:
    void colorChanged(QColor _t1);
    void accountLabelChanged();

private:
    QColor m_color;

    void polishColor();
};
