// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <Quotient/jobs/basejob.h>

// TODO: Remove once libQuotient updates to Matrix API v1.14
class NeochatReportUserJob : public Quotient::BaseJob
{
public:
    explicit NeochatReportUserJob(const QString &userId, const QString &reason);
};
