import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1

Dialog {
    anchors.centerIn: parent

    width: 480

    id: root

    contentItem: Column {
        id: detailColumn

        spacing: 0

        ListView {
            width: parent.width
            height: 48

            clip: true

            orientation: ListView.Horizontal

            spacing: 16

            model: AccountListModel{
                controller: spectralController
            }

            delegate: Avatar {
                width: 48
                height: 48

                source: user.avatarMediaId
                hint: user.displayName || "No Name"

                Menu {
                    id: contextMenu

                    MenuItem {
                        text: "Logout"

                        onClicked: spectralController.logout(connection)
                    }
                }

                RippleEffect {
                    anchors.fill: parent

                    circular: true

                    onPrimaryClicked: spectralController.connection = connection
                    onSecondaryClicked: contextMenu.popup()
                }
            }
        }

        RowLayout {
            width: parent.width

            MenuSeparator {
                Layout.fillWidth: true
            }

            ToolButton {
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48

                contentItem: MaterialIcon {
                    icon: "\ue145"
                    color: MPalette.lighter
                }

                onClicked: loginDialog.createObject(ApplicationWindow.overlay).open()
            }
        }

        Control {
            width: parent.width

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    color: MPalette.foreground
                    icon: "\ue7ff"
                }

                Label {
                    Layout.fillWidth: true

                    color: MPalette.foreground
                    text: "Start a Chat"
                }
            }

            RippleEffect {
                anchors.fill: parent

                onPrimaryClicked: joinRoomDialog.createObject(ApplicationWindow.overlay).open()
            }
        }

        Control {
            width: parent.width

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    color: MPalette.foreground
                    icon: "\ue7fc"
                }

                Label {
                    Layout.fillWidth: true

                    color: MPalette.foreground
                    text: "Create a Room"
                }
            }

            RippleEffect {
                anchors.fill: parent

                onPrimaryClicked: createRoomDialog.createObject(ApplicationWindow.overlay).open()
            }
        }

        MenuSeparator {
            width: parent.width
        }

        Control {
            width: parent.width

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    color: MPalette.foreground
                    icon: "\ue3a9"
                }

                Label {
                    Layout.fillWidth: true

                    color: MPalette.foreground
                    text: "Night Mode"
                }

                Switch {
                    id: darkThemeSwitch

                    checked: MSettings.darkTheme
                    onCheckedChanged: MSettings.darkTheme = checked
                }
            }

            RippleEffect {
                anchors.fill: parent

                onPrimaryClicked: darkThemeSwitch.checked = !darkThemeSwitch.checked
            }
        }

        Control {
            width: parent.width

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    color: MPalette.foreground
                    icon: "\ue8f8"
                }

                Label {
                    Layout.fillWidth: true

                    color: MPalette.foreground
                    text: "Show Join/Leave"
                }

                Switch {
                    id: showJoinLeaveSwitch

                    checked: MSettings.value("UI/show_joinleave", true)
                    onCheckedChanged: MSettings.setValue("UI/show_joinleave", checked)
                }
            }

            RippleEffect {
                anchors.fill: parent

                onPrimaryClicked: showJoinLeaveSwitch.checked = !showJoinLeaveSwitch.checked
            }
        }

        Control {
            width: parent.width

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    color: MPalette.foreground
                    icon: "\ue5d2"
                }

                Label {
                    Layout.fillWidth: true

                    color: MPalette.foreground
                    text: "Enable System Tray"
                }

                Switch {
                    id: trayIconSwitch

                    checked: MSettings.showTray
                    onCheckedChanged: MSettings.showTray = checked
                }
            }

            RippleEffect {
                anchors.fill: parent

                onPrimaryClicked: trayIconSwitch.checked = !trayIconSwitch.checked
            }
        }

        Control {
            width: parent.width

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    color: MPalette.foreground
                    icon: "\ue7f5"
                }

                Label {
                    Layout.fillWidth: true

                    color: MPalette.foreground
                    text: "Enable Notifications"
                }

                Switch {
                    id: notificationsSwitch

                    checked: MSettings.showNotification
                    onCheckedChanged: MSettings.showNotification = checked
                }
            }

            RippleEffect {
                anchors.fill: parent

                onPrimaryClicked: notificationsSwitch.checked = !notificationsSwitch.checked
            }
        }

        MenuSeparator {
            width: parent.width
        }

        Control {
            width: parent.width

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    color: MPalette.foreground
                    icon: "\ue167"
                }

                Label {
                    Layout.fillWidth: true

                    color: MPalette.foreground
                    text: "Font Family"
                }
            }

            RippleEffect {
                anchors.fill: parent

                onPrimaryClicked: fontFamilyDialog.createObject(ApplicationWindow.overlay).open()
            }
        }
    }

    onClosed: destroy()
}
