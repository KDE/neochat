import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3
import Matrique 0.1
import Matrique.Settings 0.1

import "component"
import "form"

Page {
    property alias listModel: accountSettingsListView.model
    Page {
        id: accountForm
        parent: null

        padding: 64

        ListView {
            anchors.fill: parent

            id: accountSettingsListView

            spacing: 0

            delegate: RowLayout{
                Label {
                    text: accountID
                }
                ItemDelegate {
                    text: "Logout"
                    onClicked: matriqueController.logout(connection);
                }
            }
        }
    }

    Page {
        id: generalForm

        parent: null

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

    RowLayout {
        ColumnLayout {
            Layout.preferredWidth: 240
            Layout.fillHeight: true

            spacing: 0

            ItemDelegate {
                Layout.fillWidth: true

                text: "Account"
                onClicked: pushToStack(accountForm)
            }

            ItemDelegate {
                Layout.fillWidth: true

                text: "General"
                onClicked: pushToStack(generalForm)
            }

            ItemDelegate {
                Layout.fillWidth: true

                text: "Appearance"
                onClicked: pushToStack(appearanceForm)
            }

            ItemDelegate {
                Layout.fillWidth: true

                text: "About"
                onClicked: pushToStack(aboutForm)
            }
        }

        StackView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: settingStackView
        }
    }

    function pushToStack(item) {
        settingStackView.clear()
        settingStackView.push(item)
    }
}
