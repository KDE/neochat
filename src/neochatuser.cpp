// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "neochatuser.h"

#include <QGuiApplication>
#include <QJsonObject>
#include <QPalette>

#include <connection.h>

NeoChatUser::NeoChatUser(QString userId, Connection *connection)
    : User(std::move(userId), connection)
{
    connect(static_cast<QGuiApplication *>(QGuiApplication::instance()), &QGuiApplication::paletteChanged, this, &NeoChatUser::polishColor);
    polishColor();
    if (connection->userId() == id()) {
        connect(connection, &Connection::accountDataChanged, this, [this](QString type) {
            if (type == QLatin1String("org.kde.neochat.account_label")) {
                Q_EMIT accountLabelChanged();
            }
        });
    }
}

QColor NeoChatUser::color()
{
    return m_color;
}

void NeoChatUser::setColor(const QColor &color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;
    Q_EMIT colorChanged(m_color);
}

void NeoChatUser::polishColor()
{
    const auto lightness = static_cast<QGuiApplication *>(QGuiApplication::instance())->palette().color(QPalette::Active, QPalette::Window).lightnessF();
    // https://github.com/quotient-im/libQuotient/wiki/User-color-coding-standard-draft-proposal
    setColor(QColor::fromHslF(hueF(), 1, -0.7 * lightness + 0.9, 1));
}

void NeoChatUser::setAccountLabel(const QString &accountLabel)
{
    Q_ASSERT(connection()->user()->id() == id());
    QJsonObject json;
    json["account_label"] = accountLabel;
    connection()->setAccountData("org.kde.neochat.account_label", json);
}

QString NeoChatUser::accountLabel() const
{
    Q_ASSERT(connection()->user()->id() == id());
    return connection()->accountDataJson("org.kde.neochat.account_label")["account_label"].toString();
}
