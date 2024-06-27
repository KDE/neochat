// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#if Quotient_VERSION_MINOR > 8
#define Omittable std::optional
#define none Quotient::none
#else
#define Omittable Quotient::Omittable
#define none std::nullopt
#endif
