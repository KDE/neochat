import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.4 as Kirigami

import Spectral 0.1
import Spectral.Setting 0.1
import Spectral.Component 2.0

RowLayout {
    default property alias innerObject : column.children

    readonly property bool sentByMe: author.isLocalUser
    readonly property bool replyVisible: reply || false
    readonly property bool failed: marks === EventStatus.SendingFailed

    id: root

    spacing: Kirigami.Units.largeSpacing

    Avatar {
        Layout.preferredWidth: Kirigami.Units.gridUnit * 1.5
        Layout.preferredHeight: Kirigami.Units.gridUnit * 1.5

        Layout.alignment: Qt.AlignTop

        visible: showAuthor
        hint: author.displayName
        source: author.avatarMediaId
        color: author.color
    }

    Item {
        Layout.preferredWidth: Kirigami.Units.gridUnit * 1.5
        Layout.preferredHeight: 1

        visible: !showAuthor
    }

    ColumnLayout {
        Layout.fillWidth: true

        id: column

        spacing: Kirigami.Units.smallSpacing

        Controls.Label {
            Layout.fillWidth: true

            visible: showAuthor

            text: author.displayName
            font.bold: true
            color: Kirigami.Theme.activeTextColor
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.fillWidth: true

            visible: replyVisible

            Rectangle {
                Layout.preferredWidth: 4
                Layout.fillHeight: true

                color: Kirigami.Theme.highlightColor
            }

            Avatar {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 1.5
                Layout.preferredHeight: Kirigami.Units.gridUnit * 1.5
                Layout.alignment: Qt.AlignTop

                source: replyVisible ? reply.author.avatarMediaId : ""
                hint: replyVisible ? reply.author.displayName : "H"
                color: replyVisible ? reply.author.color : MPalette.accent
            }

            ColumnLayout {
                Layout.fillWidth: true

                Controls.Label {
                    Layout.fillWidth: true

                    text: replyVisible ? reply.author.displayName : ""
                    color: Kirigami.Theme.activeTextColor
                    wrapMode: Text.Wrap
                }

                Text {
                    Layout.fillWidth: true

                    text: "<style>pre {white-space: pre-wrap} a{color: " + color + ";} .user-pill{}</style>" + (replyVisible ? reply.display : "")

                    color: Kirigami.Theme.textColor
//                    selectionColor: Kirigami.Theme.highlightColor
//                    selectedTextColor: Kirigami.Theme.highlightedTextColor

//                    selectByMouse: true
//                    readOnly: true
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    textFormat: Text.RichText
                }
            }
        }
    }
}
