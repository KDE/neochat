import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import Spectral.Component 2.0
import Spectral.Setting 0.1

Control {
    padding: 8

    contentItem: RowLayout {
        id: row

        Control {
            id: authorControl

            padding: 4

            background: Rectangle {
                radius: height / 2
                color: author.color
            }

            contentItem: RowLayout {
                Avatar {
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24

                    hint: author.displayName
                    source: author.avatarMediaId
                }

                Label {
                    Layout.alignment: Qt.AlignVCenter

                    text: author.displayName
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    color: "white"
                    rightPadding: 8
                }
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.maximumWidth: messageListView.width - authorControl.width - row.spacing

            text: display + " • " + Qt.formatTime(time, "hh:mm AP")
            color: MPalette.foreground
            font.pixelSize: 13
            font.weight: Font.Medium

            wrapMode: Label.Wrap
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
