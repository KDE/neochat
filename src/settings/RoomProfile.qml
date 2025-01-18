// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitemmodels

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property NeoChatRoom room

    title: i18nc('@title:window', 'Profile')

    function setDisplayName(): void {
        root.room.connection.localUser.rename(displayNameDelegate.text, root.room);
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextDelegate {
            id: noticeLabel
            text: i18nc("@info", "Customize your profile only for this room.")
        }
        FormCard.FormDelegateSeparator {
            above: noticeLabel
            below: displayNameDelegate
        }
        FormCard.FormTextFieldDelegate {
            id: displayNameDelegate
            label: i18nc("@label:textbox", "Display Name")
            text: root.room.member(root.room.connection.localUserId).displayName
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 2

        FormCard.FormButtonDelegate {
            icon.name: "document-save-symbolic"
            text: i18nc("@action:button Save profile", "Save")
            onClicked: root.setDisplayName()
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            icon.name: "kt-restore-defaults-symbolic"
            text: i18nc("@action:button", "Reset to Default")
            onClicked: {
                displayNameDelegate.text = root.room.connection.localUser.displayName;
                root.setDisplayName();
            }
        }
    }
}
