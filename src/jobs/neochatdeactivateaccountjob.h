// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <Quotient/jobs/basejob.h>

class NeoChatDeactivateAccountJob : public Quotient::BaseJob
{
public:
    explicit NeoChatDeactivateAccountJob(const std::optional<QJsonObject> &auth = std::nullopt);
};
