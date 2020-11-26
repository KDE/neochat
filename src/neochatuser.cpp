/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "neochatuser.h"

#include <PlatformTheme> // Kirigami
#include <utility>

#include "csapi/profile.h"

NeoChatUser::NeoChatUser(QString userId, Connection *connection)
    : User(std::move(userId), connection)
{
    m_theme = static_cast<Kirigami::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::PlatformTheme>(this, true));
    Q_ASSERT(m_theme);

    connect(m_theme, &Kirigami::PlatformTheme::colorsChanged, this, &NeoChatUser::polishColor);
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
    emit colorChanged(m_color);
}

void NeoChatUser::polishColor()
{
    // https://github.com/quotient-im/libQuotient/wiki/User-color-coding-standard-draft-proposal
    setColor(QColor::fromHslF(hueF(), 1 - m_theme->alternateBackgroundColor().saturationF(), -0.7 * m_theme->alternateBackgroundColor().lightnessF() + 0.9, m_theme->textColor().alphaF()));
}
