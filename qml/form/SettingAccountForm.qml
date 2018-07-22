import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import "qrc:/qml/component"

Page {
    padding: 64

    ColumnLayout {
        RowLayout {
            Layout.preferredHeight: 60

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
                    font.pointSize: 18
                    text: connection ? connection.localUser.displayName : ""
                }

                Label {
                    font.pointSize: 12
                    text: connection ? connection.localUser.id : ""
                }
            }
        }

        Button {
           text: "Logout"
           highlighted: true

           onClicked: matriqueController.logout()
        }
    }
}
