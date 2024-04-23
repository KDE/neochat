// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.neochat

Kirigami.Dialog {
    id: root

    title: i18nc("@title", "Incoming call")

    width: Kirigami.Units.gridUnit * 16
    height: Kirigami.Units.gridUnit * 8

    standardButtons: QQC2.Dialog.NoButton

    Connections {
        target: MediaManager
        function onCloseIncomingCallDialog() {
            root.close()
        }
    }

    contentItem: ColumnLayout {
        Components.DoubleFloatingButton {
            anchors.centerIn: parent
            leadingAction: Kirigami.Action {
                icon.name: "call-start"
                text: i18nc("@action:button", "Accept Call")
                tooltip: ""//text
            }
            trailingAction: Kirigami.Action {
                icon.name: "call-stop"
                text: i18nc("@action:button", "Decline Call")
                tooltip: ""//text
            }
        }
    }

}
