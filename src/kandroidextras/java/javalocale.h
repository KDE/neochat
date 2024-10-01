/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_LOCALE_H
#define KANDROIDEXTRAS_LOCALE_H

#include "kandroidextras_export.h"

class QJniObject;
class QLocale;

namespace KAndroidExtras
{

/** Conversion methods between java.util.Locale and QLocale.
 *  @note Do not rename this file to locale.h, that clashes with POSIX locale.h when your
 *  include paths are unfortunately set up causing bizarre compilation issues.
 */
namespace Locale
{
/** Create an java.util.Locale object from a QLocale. */
KANDROIDEXTRAS_EXPORT QJniObject fromLocale(const QLocale &locale);

/** Create an java.util.Locale object for the current QLocale. */
KANDROIDEXTRAS_EXPORT QJniObject current();
}

}

#endif // KANDROIDEXTRAS_LOCALE_H
