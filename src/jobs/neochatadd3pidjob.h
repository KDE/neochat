// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <Quotient/jobs/basejob.h>
#include <Quotient/omittable.h>

class NeochatAdd3PIdJob : public Quotient::BaseJob
{
public:
    explicit NeochatAdd3PIdJob(const QString &clientSecret, const QString &sid, const Quotient::Omittable<QJsonObject> &auth = {});
};
