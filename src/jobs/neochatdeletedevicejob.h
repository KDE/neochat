// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <Quotient/jobs/basejob.h>
#include <Quotient/omittable.h>

class NeochatDeleteDeviceJob : public Quotient::BaseJob
{
public:
    explicit NeochatDeleteDeviceJob(const QString &deviceId, const Quotient::Omittable<QJsonObject> &auth = Quotient::none);
};
