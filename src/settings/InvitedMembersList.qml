// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.neochat
import io.github.quotient_im.libquotient

MembersList {
    id: invitedMembersPage

    title: i18nc("@title", "Invited Members")
    membership: Quotient.MembershipMask.Invite
    room: root.room
    confirmationTitle: i18nc("@title:dialog", "Uninvite User")
    confirmationSubtitle: i18nc("@info %1 is a matrix ID", "Do you really want to uninvite %1?", currentMemberId)
    icon: "im-ban-kick-user-symbolic"
    actionText: i18nc("@action:button", "Uninvite…")
    actionConfirmationText: i18nc("@action:button", "Uninvite")
    actionVisible: root.room.canSendState("kick")

    onActionTaken: memberId => root.room.kickMember(memberId, "Revoked invite")
}
