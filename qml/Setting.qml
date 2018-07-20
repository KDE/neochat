import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3

import "component"

Page {
    property alias darkTheme: themeSwitch.checked
    property alias miniMode: miniModeSwitch.checked
    property var connection

    header: TabBar {
        id: tabBar
        width: parent.width
        currentIndex: settingView.currentIndex

        TabButton {
            text: "Account"

        }
        TabButton {
            text: "Appearance"
        }
        TabButton {
            text: "About"
        }
    }

    SwipeView {
        id: settingView

        currentIndex: tabBar.currentIndex
        anchors.fill: parent

        Item {
            id: accountPage

            RowLayout {
                height: 80

                ImageStatus {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    source: connection ? connection.localUser && connection.localUser.avatarUrl ? "image://mxc/" + connection.localUser.avatarUrl : "" : "qrc:/asset/img/avatar.png"
                    displayText: connection && connection.localUser.displayName ? connection.localUser.displayName : "N"
                    opaqueBackground: false
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Label {
                        font.pointSize: 24
                        text: connection ? connection.localUser.displayName : ""
                    }

                    Label {
                        font.pointSize: 16
                        text: "No text."
                    }
                }
            }
        }

        Item {
            id: appearancePage

            Column {
                Switch {
                    id: themeSwitch
                    text: "Dark theme"
                }

                Switch {
                    id: miniModeSwitch
                    text: "Mini Room List"
                }
            }
        }

        Item {
            id: thirdPage
        }
    }
}
