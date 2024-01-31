// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat

Kirigami.Dialog {
    id: root

    /**
     * @brief The current room this dialog is opened for.
     */
    required property NeoChatRoom room

    /**
     * @brief The current list of space IDs that members of can join this room.
     */
    property list<string> restrictedIds: room.restrictedIds

    parent: applicationWindow().overlay

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    title: i18nc("@title", "Select Spaces")

    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
    onAccepted: {
        let ids = [];
        for (var i in spaceGroup.buttons) {
            if (spaceGroup.buttons[i].checked) {
                ids.push(spaceGroup.buttons[i].modelData.id);
            }
        }
        root.room.setJoinRule("restricted", ids);
        console.warn(ids);
    }

    QQC2.ButtonGroup {
        id: spaceGroup
        exclusive: false
    }

    contentItem: ColumnLayout {
        spacing: 0
        Repeater {
            model: root.room.parentObjects(true)

            delegate: FormCard.FormCheckDelegate {
                required property var modelData

                text: modelData.displayName
                description: modelData.canonicalAlias
                checked: root.restrictedIds.includes(modelData.id)
                QQC2.ButtonGroup.group: spaceGroup

                leading: Components.Avatar {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                    source: modelData.avatarUrl.toString().length > 0 ? connection.makeMediaUrl(modelData.avatarUrl) : ""
                    name: modelData.displayName
                }
            }
        }
    }
}
