/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "neochatuser.h"

#include <PlatformTheme> // Kirigami
#include <utility>

#include "csapi/profile.h"

static Kirigami::PlatformTheme * s_theme = nullptr;

NeoChatUser::NeoChatUser(QString userId, Connection *connection)
    : User(std::move(userId), connection)
{
    if (!s_theme) {
        s_theme = static_cast<Kirigami::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::PlatformTheme>(this, true));
        Q_ASSERT(s_theme);
    }

    connect(s_theme, &Kirigami::PlatformTheme::colorsChanged, this, &NeoChatUser::polishColor);
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
    // https://github.com/quotient-im/libQuotient/wiki/User-color-coding-standard-draft-proposal
    setColor(QColor::fromHslF(hueF(), 1 - s_theme->alternateBackgroundColor().saturationF(), -0.7 * s_theme->alternateBackgroundColor().lightnessF() + 0.9, s_theme->textColor().alphaF()));
}
