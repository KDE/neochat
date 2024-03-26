// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat
import org.kde.neochat.accounts

ListView {
    id: root

    required property NeoChatConnection connection

    property bool inDialog: false

    property var addAccount

    implicitHeight: contentHeight

    Kirigami.Theme.colorSet: inDialog ? Kirigami.Theme.View : Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    footer: Delegates.RoundedItemDelegate {
        id: addDelegate
        width: parent.width
        highlighted: focus && !addAccount.pressed
        Component.onCompleted: root.addAccount = this
        icon {
            name: "list-add"
            width: Kirigami.Units.iconSizes.smallMedium
            height: Kirigami.Units.iconSizes.smallMedium
        }
        text: i18n("Add Account")
        contentItem: Delegates.SubtitleContentItem {
            itemDelegate: parent
            subtitle: i18n("Log in to an existing account")
            labelItem.textFormat: Text.PlainText
            subtitleItem.textFormat: Text.PlainText
        }

        onClicked: {
            pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'WelcomePage.qml'), {}, {
                title: i18nc("@title:window", "Login")
            });
            if (switchUserButton.checked) {
                switchUserButton.checked = false;
            }
            root.currentIndex = Controller.activeConnectionIndex;
        }
        Keys.onUpPressed: {
            root.currentIndex = root.count - 1;
            root.forceActiveFocus();
        }
        Keys.onDownPressed: {
            root.currentIndex = 0;
            root.forceActiveFocus();
        }
    }
    clip: true
    model: AccountRegistry

    keyNavigationEnabled: false
    Keys.onDownPressed: {
        if (root.currentIndex === root.count - 1) {
            addAccount.forceActiveFocus();
            root.currentIndex = -1;
        } else {
            root.incrementCurrentIndex();
        }
    }
    Keys.onUpPressed: {
        if (root.currentIndex === 0) {
            addAccount.forceActiveFocus();
            root.currentIndex = -1;
        } else {
            root.decrementCurrentIndex();
        }
    }

    Keys.onReleased: if (event.key == Qt.Key_Escape) {
        if (switchUserButton.checked) {
            switchUserButton.checked = false;
        }
    }

    onVisibleChanged: {
        for (let i = 0; i < root.count; i++) {
            if (model.data(model.index(i, 0), Qt.DisplayRole) === root.connection.localUser.id) {
                root.currentIndex = i;
                break;
            }
        }
    }

    delegate: Delegates.RoundedItemDelegate {
        id: userDelegate

        required property NeoChatConnection connection

        width: parent.width
        text: connection.localUser.displayName

        contentItem: RowLayout {
            KirigamiComponents.Avatar {
                implicitWidth: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                implicitHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                sourceSize {
                    width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                    height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                }
                source: userDelegate.connection.localUser.avatarMediaId ? ("image://mxc/" + userDelegate.connection.localUser.avatarMediaId) : ""
                name: userDelegate.connection.localUser.displayName ?? userDelegate.connection.localUser.id
            }

            Delegates.SubtitleContentItem {
                itemDelegate: userDelegate
                subtitle: userDelegate.connection.localUser.id
                labelItem.textFormat: Text.PlainText
                subtitleItem.textFormat: Text.PlainText
            }
        }

        onClicked: {
            Controller.activeConnection = userDelegate.connection;
            if (switchUserButton.checked) {
                switchUserButton.checked = false;
            }
        }
    }
}
