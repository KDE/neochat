// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.neochat
import io.github.quotient_im.libquotient

MembersList {
    id: bannedMembersPage

    title: i18nc("@title", "Banned Members")
    membership: Quotient.MembershipMask.Ban
    room: root.room
    confirmationTitle: i18nc("@title:dialog", "Unban User")
    confirmationSubtitle: i18nc("@info %1 is a matrix ID", "Do you really want to unban %1?", currentMemberId)
    icon: "checkmark-symbolic"
    actionText: i18nc("@action:button", "Unban…")
    actionConfirmationText: i18nc("@action:button", "Unban")
    actionVisible: root.room.canSendState("ban")

    onActionTaken: memberId => root.room.unban(memberId)
}
