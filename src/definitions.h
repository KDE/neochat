// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#if Quotient_VERSION_MINOR > 8
#define Omittable std::optional
#define quotientNone std::nullopt
#else
#include <Quotient/omittable.h>
#define Omittable Quotient::Omittable
#define quotientNone Quotient::none
#endif
