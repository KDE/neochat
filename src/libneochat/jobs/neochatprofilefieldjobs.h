// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <Quotient/jobs/basejob.h>

// NOTE: This is currently being upstreamed to libQuotient, awaiting MSC4133: https://github.com/quotient-im/libQuotient/pull/869

//! \brief Get a user's profile field.
//!
//! Get one of the user's profile fields. This API may be used to fetch the user's
//! own profile field or to query the profile field of other users; either locally or
//! on remote homeservers.
class QUOTIENT_API NeoChatGetProfileFieldJob : public Quotient::BaseJob
{
public:
    //! \param userId
    //!   The user whose profile field to query.
    //! \param key
    //!   The key of the profile field.
    explicit NeoChatGetProfileFieldJob(const QString &userId, const QString &key);

    // Result properties

    //! The value of the profile field.
    QString value() const
    {
        return loadFromJson<QString>(m_key);
    }

private:
    QString m_key;
};

//! \brief Sets a user's profile field.
//!
//! Set one of the user's own profile fields. This may fail depending on if the server allows the
//! user to change their own profile field, or if the field isn't allowed.
class QUOTIENT_API NeoChatSetProfileFieldJob : public Quotient::BaseJob
{
public:
    //! \param userId
    //!   The user whose avatar URL to set.
    //!
    //! \param avatarUrl
    //!   The new avatar URL for this user.
    explicit NeoChatSetProfileFieldJob(const QString &userId, const QString &key, const QString &value);
};
