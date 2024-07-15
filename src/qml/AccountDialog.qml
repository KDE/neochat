// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.prison

import org.kde.neochat

Kirigami.Dialog {
    id: root

    property NeoChatConnection connection

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    standardButtons: Kirigami.Dialog.NoButton

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)

    contentItem: ColumnLayout {
        spacing: 0

        KirigamiComponents.Avatar {
            id: avatar

            readonly property string mediaId: root.connection.localUser.avatarMediaId

            Layout.preferredWidth: Kirigami.Units.iconSizes.huge * 2
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge * 2
            Layout.alignment: Qt.AlignHCenter

            name: root.room ? root.room.member(root.user.id).displayName : root.user.displayName
            source: mediaId ? root.connection.makeMediaUrl("mxc://" + mediaId) : ""
            color: root.room ? root.room.member(root.user.id).color : QmlUtils.getUserColor(root.user.hueF)
        }

        QQC2.Label {
            Layout.fillWidth: true
            font.bold: false
            font.weight: Font.DemiBold
            font.pixelSize: 32

            elide: Text.ElideRight
            wrapMode: Text.NoWrap
            text: root.connection.localUser.displayName
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignHCenter
        }

        Kirigami.SelectableLabel {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            font.pixelSize: 16
            textFormat: TextEdit.PlainText
            text: root.connection.localUser.id
            horizontalAlignment: Text.AlignHCenter
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18n("Edit Account")
        }
        FormCard.FormButtonDelegate {
            text: i18n("Switch Account")
        }
        FormCard.FormButtonDelegate {
            text: i18n("Logout")
        }

    }
    Component {
        id: qrMaximizeComponent
        QrCodeMaximizeComponent {}
    }
}
