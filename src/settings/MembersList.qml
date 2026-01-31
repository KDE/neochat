// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as KirigamiComponents
import org.kde.kitemmodels

import org.kde.neochat


FormCard.FormCardPage {
    id: root

    required property NeoChatRoom room
    property alias membership: userFilterModel.membership
    property alias confirmationTitle: actionDialog.title
    property alias confirmationSubtitle: actionDialog.subtitle
    property string currentMemberId
    required property string icon
    required property string actionText
    required property string actionConfirmationText
    required property bool actionVisible

    signal actionTaken(memberId: string)

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4
        Layout.fillWidth: true

        FormCard.FormPlaceholderMessageDelegate {
            text: i18nc("@info:placeholder", "No members")
            visible: userRepeater.count === 0
        }

        Repeater {
            id: userRepeater

            model: UserFilterModel {
                id: userFilterModel
                sourceModel: RoomManager.userListModel
                allowEmpty: true
            }

            delegate: FormCard.FormTextDelegate {
                id: userDelegate

                required property string userId

                text: userId
                textItem.textFormat: Text.PlainText

                contentItem.children: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    QQC2.Button {
                        icon.name: root.icon
                        visible: root.actionVisible
                        text: root.actionText

                        onClicked: {
                            root.currentMemberId = userDelegate.userId;
                            actionDialog.open();
                        }
                    }
                }
            }
        }
    }

    Kirigami.PromptDialog {
        id: actionDialog

        parent: root.QQC2.Overlay.overlay
        footer: QQC2.DialogButtonBox {
            standardButtons: QQC2.Dialog.Cancel

            QQC2.Button {
                icon.name: root.icon
                text: root.actionConfirmationText

                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            }
        }

        onAccepted: root.actionTaken(root.currentMemberId)
    }
}
