import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3
import Matrique.Settings 0.1

import "component"
import "form"

Page {
//    Page {
//        id: accountForm
//        parent: null

//        padding: 64

//        ColumnLayout {
//            RowLayout {
//                Layout.preferredHeight: 60

//                ImageStatus {
//                    Layout.preferredWidth: height
//                    Layout.fillHeight: true

//                    source: matriqueController.isLogin ? connection.localUser && connection.localUser.avatarUrl ? "image://mxc/" + connection.localUser.avatarUrl : "" : "qrc:/asset/img/avatar.png"
//                    displayText: matriqueController.isLogin && connection.localUser.displayName ? connection.localUser.displayName : ""
//                }

//                ColumnLayout {
//                    Layout.fillWidth: true
//                    Layout.fillHeight: true

//                    Label {
//                        font.pointSize: 18
//                        text: matriqueController.isLogin ? connection.localUser.displayName : ""
//                    }

//                    Label {
//                        font.pointSize: 12
//                        text: matriqueController.isLogin ? connection.localUser.id : ""
//                    }
//                }
//            }

//            Button {
//                text: "Logout"
//                highlighted: true

//                onClicked: {
//                    matriqueController.logout()
//                    Qt.quit()
//                }
//            }
//        }
//    }

    Page{
        id: accountForm

        parent: null

//        Button {
//            flat: true
//            highlighted: true
//            text: "Login"

//            onClicked: stackView.push(loginPage)
//        }
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
