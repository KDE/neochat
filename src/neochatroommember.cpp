// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "neochatroommember.h"

#include "neochatroom.h"

NeochatRoomMember::NeochatRoomMember(NeoChatRoom *room, const QString &memberId)
    : m_room(room)
    , m_memberId(memberId)
{
    Q_ASSERT(!m_memberId.isEmpty());

    if (m_room != nullptr) {
        connect(m_room, &NeoChatRoom::memberNameUpdated, this, [this](Quotient::RoomMember member) {
            if (member.id() == m_memberId) {
                Q_EMIT displayNameUpdated();
            }
        });
        connect(m_room, &NeoChatRoom::memberAvatarUpdated, this, [this](Quotient::RoomMember member) {
            if (member.id() == m_memberId) {
                Q_EMIT avatarUpdated();
            }
        });
    }
}

QString NeochatRoomMember::id() const
{
    return m_memberId;
}

Quotient::Uri NeochatRoomMember::uri() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return {};
    }

    return m_room->member(m_memberId).uri();
}

bool NeochatRoomMember::isLocalMember() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return false;
    }

    return m_room->member(m_memberId).isLocalMember();
}

Quotient::Membership NeochatRoomMember::membershipState() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return Quotient::Membership::Leave;
    }

    return m_room->member(m_memberId).membershipState();
}

QString NeochatRoomMember::name() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return id();
    }

    return m_room->member(m_memberId).name();
}

QString NeochatRoomMember::displayName() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return id();
    }

    const auto memberObject = m_room->member(m_memberId);
    return memberObject.isEmpty() ? id() : memberObject.displayName();
}

QString NeochatRoomMember::htmlSafeDisplayName() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return id();
    }

    const auto memberObject = m_room->member(m_memberId);
    return memberObject.isEmpty() ? id() : memberObject.htmlSafeDisplayName();
}

QString NeochatRoomMember::fullName() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return id();
    }

    const auto memberObject = m_room->member(m_memberId);
    return memberObject.isEmpty() ? id() : memberObject.fullName();
}

QString NeochatRoomMember::htmlSafeFullName() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return id();
    }

    const auto memberObject = m_room->member(m_memberId);
    return memberObject.isEmpty() ? id() : memberObject.htmlSafeFullName();
}

QString NeochatRoomMember::disambiguatedName() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return id();
    }

    const auto memberObject = m_room->member(m_memberId);
    return memberObject.isEmpty() ? id() : memberObject.disambiguatedName();
}

QString NeochatRoomMember::htmlSafeDisambiguatedName() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return id();
    }

    const auto memberObject = m_room->member(m_memberId);
    return memberObject.isEmpty() ? id() : memberObject.htmlSafeDisambiguatedName();
}

int NeochatRoomMember::hue() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return 0;
    }

    return m_room->member(m_memberId).hue();
}

qreal NeochatRoomMember::hueF() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return 0.0;
    }

    return m_room->member(m_memberId).hueF();
}

QColor NeochatRoomMember::color() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return {};
    }

    return m_room->member(m_memberId).color();
}

QUrl NeochatRoomMember::avatarUrl() const
{
    if (m_room == nullptr || m_memberId.isEmpty()) {
        return {};
    }

    return m_room->member(m_memberId).avatarUrl();
}
