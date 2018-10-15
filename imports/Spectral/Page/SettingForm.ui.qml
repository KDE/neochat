import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3

import Spectral.Component 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1

Page {
    property alias listModel: accountSettingsListView.model

    property alias addAccountButton: addAccountButton

    implicitWidth: 400
    implicitHeight: 300

    Page {
        id: accountForm

        parent: null

        padding: 64

        ColumnLayout {
            anchors.fill: parent

            AutoListView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                id: accountSettingsListView

                boundsBehavior: Flickable.DragOverBounds

                clip: true

                delegate: SettingAccountDelegate {}
            }

            Button {
                Layout.fillWidth: true

                id: addAccountButton

                text: "Add Account"
                flat: true
                highlighted: true
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

            Switch {
                text: "Confirm on Exit"
                checked: MSettings.confirmOnExit

                onCheckedChanged: MSettings.confirmOnExit = checked
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

                source: "qrc:/assets/img/icon.png"
            }
            Label {
                text: "Spectral, an IM client for the Matrix protocol."
            }
            Label {
                text: "Released under GNU General Public License, version 3."
            }
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

            Repeater {
                model: ListModel {
                    ListElement {
                        category: "Accounts"
                        form: 0
                    }
                    ListElement {
                        category: "General"
                        form: 1
                    }
                    ListElement {
                        category: "Appearance"
                        form: 2
                    }
                    ListElement {
                        category: "About"
                        form: 3
                    }
                }

                delegate: SettingCategoryDelegate {
                    width: parent.width
                }
            }
        }
    }

    StackView {
        anchors.fill: parent
        anchors.leftMargin: settingDrawer.width

        id: settingStackView

        initialItem: aboutForm
    }
}


/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
