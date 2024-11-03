// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <Quotient/jobs/basejob.h>

class NeochatDeleteDeviceJob : public Quotient::BaseJob
{
public:
    explicit NeochatDeleteDeviceJob(const QString &deviceId, const std::optional<QJsonObject> &auth = {});
};
