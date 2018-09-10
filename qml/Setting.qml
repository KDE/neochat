import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3
import Matrique 0.1
import Matrique.Settings 0.1

import "component"
import "form"
import "qrc:/js/util.js" as Util

Page {
    property alias listModel: accountSettingsListView.model

    Page {
        id: accountForm

        parent: null

        padding: 64

        ColumnLayout {
            anchors.fill: parent

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                id: accountSettingsListView

                delegate: Column {
                    SwipeDelegate {
                        width: accountSettingsListView.width
                        height: 64

                        clip: true

                        Row {
                            anchors.fill: parent
                            anchors.margins: 8

                            spacing: 8

                            ImageItem {
                                width: parent.height
                                height: parent.height

                                hint: user.displayName
                                defaultColor: Util.stringToColor(user.displayName)
                                image: user.avatar
                            }

                            ColumnLayout {
                                Label {
                                    text: user.displayName
                                }
                                Label {
                                    text: user.id
                                }
                            }
                        }

                        swipe.right: Rectangle {
                            width: parent.height
                            height: parent.height
                            anchors.right: parent.right

                            color: Material.accent

                            MaterialIcon {
                                anchors.fill: parent

                                icon: "\ue879"
                                color: "white"
                            }

                            SwipeDelegate.onClicked: matriqueController.logout(connection)
                        }

                        onClicked: accountSettingsListView.currentIndex == index ? accountSettingsListView.currentIndex = -1 : accountSettingsListView.currentIndex = index
                    }

                    Rectangle {
                        width: parent.width
                        height: 2
                        visible: accountSettingsListView.currentIndex == index

                        color: Material.accent
                    }

                    ColumnLayout {
                        visible: accountSettingsListView.currentIndex == index
                        width: parent.width - 32
                        anchors.horizontalCenter: parent.horizontalCenter

                        RowLayout {
                            Layout.fillWidth: true

                            spacing: 16

                            Label { text: "Homeserver:" }
                            TextField {
                                Layout.fillWidth: true

                                text: connection.homeserver
                                selectByMouse: true
                                readOnly: true
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            spacing: 16

                            Label { text: "Device ID:" }
                            TextField {
                                Layout.fillWidth: true

                                text: connection.deviceId
                                selectByMouse: true
                                readOnly: true
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            spacing: 16

                            Label { text: "Access Token:" }
                            TextField {
                                Layout.fillWidth: true

                                text: connection.accessToken
                                selectByMouse: true
                                readOnly: true
                            }
                        }
                    }
                }
            }

            Button {
                text: "Add Account"
                flat: true
                highlighted: true

                onClicked: Util.pushToStack(stackView, loginPage)
            }
        }
    }

    Page {
        id: generalForm

        parent: null

        padding: 64

        Column {
            Switch {
                text: "Lazy load at initial sync"
                checked: MSettings.lazyLoad

                onCheckedChanged: MSettings.lazyLoad = checked
            }

            Switch {
                text: "Use RichText instead of StyledText"
                checked: MSettings.richText

                onCheckedChanged: MSettings.richText = checked
            }

            Switch {
                text: "Use press and hold instead of right click"
                checked: MSettings.pressAndHold

                onCheckedChanged: MSettings.pressAndHold = checked
            }
        }
    }

    Page {
        id: appearanceForm

        parent: null

        padding: 64

        Column {
            Switch {
                text: "Dark theme"
                checked: MSettings.darkTheme

                onCheckedChanged: MSettings.darkTheme = checked
            }

            Switch {
                text: "Mini Room List"
                checked: MSettings.miniMode

                onCheckedChanged: MSettings.miniMode = checked
            }

            Switch {
                text: "Rearrange rooms by activity"
                checked: MSettings.rearrangeByActivity

                onCheckedChanged: MSettings.rearrangeByActivity = checked
            }
        }
    }

    Page {
        id: aboutForm

        parent: null

        padding: 64

        ColumnLayout {
            spacing: 16
            Image {
                Layout.preferredWidth: 64
                Layout.preferredHeight: 64

                source: "qrc:/asset/img/icon.png"
            }
            Label { text: "Matrique, an IM client for the Matrix protocol." }
            Label { text: "Released under GNU General Public License, version 3." }
        }
    }

    Rectangle {
        width: 240
        height: parent.height

        id: settingDrawer

        color: MSettings.darkTheme ? "#323232" : "#f3f3f3"

        Column {
            anchors.fill: parent

            ItemDelegate {
                width: parent.width

                text: "Account"
                onClicked: pushToStack(accountForm)
            }

            ItemDelegate {
                width: parent.width

                text: "General"
                onClicked: pushToStack(generalForm)
            }

            ItemDelegate {
                width: parent.width

                text: "Appearance"
                onClicked: pushToStack(appearanceForm)
            }

            ItemDelegate {
                width: parent.width

                text: "About"
                onClicked: pushToStack(aboutForm)
            }
        }
    }

    StackView {
        anchors.fill: parent
        anchors.leftMargin: settingDrawer.width

        id: settingStackView
    }

    function pushToStack(item) {
        settingStackView.clear()
        settingStackView.push(item)
    }
}
