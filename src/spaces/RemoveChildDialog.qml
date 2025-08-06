// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat.libneochat

Kirigami.Dialog {
    id: root

    required property NeoChatRoom parentRoom

    required property string roomId

    required property string displayName

    required property string parentDisplayName

    required property bool canSetParent

    required property bool isDeclaredParent

    title: i18nc("@title", "Remove Child")

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)

    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

    onAccepted: parentRoom.removeChild(root.roomId, removeOfficalCheck.checked)

    contentItem: ColumnLayout {
        spacing: 0
        FormCard.FormTextDelegate {
            text: i18n("The child %1 will be removed from the space %2", root.displayName, root.parentDisplayName)
            textItem.wrapMode: Text.Wrap
        }
        FormCard.FormCheckDelegate {
            id: removeOfficalCheck
            visible: root.isDeclaredParent
            enabled: root.canSetParent
            text: i18n("The current space is the official parent of this room, should this be cleared?")
            checked: root.canSetParent
        }
    }
}
