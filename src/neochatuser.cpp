// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "neochatuser.h"

#include <QGuiApplication>
#include <QPalette>

NeoChatUser::NeoChatUser(QString userId, Connection *connection)
    : User(std::move(userId), connection)
{
    connect(static_cast<QGuiApplication *>(QGuiApplication::instance()), &QGuiApplication::paletteChanged, this, &NeoChatUser::polishColor);
    polishColor();
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
