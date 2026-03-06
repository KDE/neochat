// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "neochatauthmetadatajob.h"

using namespace Quotient;

NeoChatAuthMetadataJob::NeoChatAuthMetadataJob()
    : BaseJob(HttpVerb::Get, u"AuthMetadataJob"_s, "/_matrix/client/v1/auth_metadata")
{
}
