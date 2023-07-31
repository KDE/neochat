// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QColor>
#include <QGuiApplication>
#include <QPalette>

namespace Utils
{

/**
 * @brief Get a color for a user from a hueF value.
 *
 * The lightness of the color will be modified depending on the current palette in
 * order to maintain contrast.
 */
inline QColor getUserColor(qreal hueF)
{
    const auto lightness = static_cast<QGuiApplication *>(QGuiApplication::instance())->palette().color(QPalette::Active, QPalette::Window).lightnessF();
    // https://github.com/quotient-im/libQuotient/wiki/User-color-coding-standard-draft-proposal
    return QColor::fromHslF(hueF, 1, -0.7 * lightness + 0.9, 1);
}
}
