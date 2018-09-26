import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3
import Spectral 0.1
import Spectral.Settings 0.1

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

                boundsBehavior: Flickable.DragOverBounds

                clip: true

                delegate: Column {
                    property bool expanded: false

                    spacing: 8

                    ItemDelegate {
                        width: accountSettingsListView.width
                        height: 64

                        Row {
                            anchors.fill: parent
                            anchors.margins: 8

                            spacing: 8

                            ImageItem {
                                width: parent.height
                                height: parent.height

                                hint: user.displayName
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

                        onClicked: expanded = !expanded
                    }

                    ColumnLayout {
                        width: parent.width - 32
                        height: expanded ? implicitHeight : 0
                        anchors.horizontalCenter: parent.horizontalCenter

                        spacing: 0

                        clip: true

                        ListView {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 24

                            orientation: ListView.Horizontal

                            spacing: 8

                            model: ["#498882", "#42a5f5", "#5c6bc0", "#7e57c2", "#ab47bc", "#ff7043"]

                            delegate: Rectangle {
                                width: parent.height
                                height: parent.height
                                radius: width / 2

                                color: modelData

                                MouseArea {
                                    anchors.fill: parent

                                    onClicked: spectralController.setColor(connection.localUserId, modelData)
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

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

                        Button {
                            Layout.fillWidth: true

                            highlighted: true
                            text: "Logout"

                            onClicked: spectralController.logout(connection)
                        }

                        Behavior on height {
                            PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
                        }
                    }
                }
            }

            Button {
                Layout.fillWidth: true

                text: "Add Account"
                flat: true
                highlighted: true

                onClicked: stackView.push(loginPage)
            }
        }
    }

    Page {
        id: generalForm

        parent: null

        padding: 64

        Column {
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
            Label { text: "Spectral, an IM client for the Matrix protocol." }
            Label { text: "Released under GNU General Public License, version 3." }
        }
    }

    Rectangle {
        width: 240
        height: parent.height
        z: 10

        id: settingDrawer

        color: MSettings.darkTheme ? "#323232" : "#f3f3f3"

        layer.enabled: true
        layer.effect: ElevationEffect {
            elevation: 4
        }

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
