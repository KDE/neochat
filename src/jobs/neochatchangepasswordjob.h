// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <Quotient/jobs/basejob.h>
#include <Quotient/omittable.h>

class NeochatChangePasswordJob : public Quotient::BaseJob
{
public:
    explicit NeochatChangePasswordJob(const QString &newPassword, bool logoutDevices, const Quotient::Omittable<QJsonObject> &auth = Quotient::none);
};
