import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

import org.kde.kirigami 2.13 as Kirigami

import Spectral 0.1
import Spectral.Setting 0.1
import Spectral.Component 2.0

RowLayout {
    default property alias innerObject : column.children

    readonly property bool sentByMe: author.isLocalUser
    readonly property bool darkBackground: !sentByMe
    readonly property bool replyVisible: reply ?? false
    readonly property bool failed: marks == EventStatus.SendingFailed
    readonly property color authorColor: eventType == "notice" ? MPalette.primary : author.color
    readonly property color replyAuthorColor: replyVisible ? reply.author.color : MPalette.accent

    signal saveFileAs()
    signal openExternally()

    id: root

    spacing: Kirigami.Units.largeSpacing
    Layout.leftMargin: Kirigami.Units.smallSpacing
    Layout.rightMargin: Kirigami.Units.smallSpacing

    Kirigami.Avatar {
        Layout.preferredWidth: Kirigami.Units.gridUnit * 2
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2

        Layout.alignment: Qt.AlignTop

        visible: showAuthor
        name: author.displayName
        source: author.avatarMediaId ? "image://mxc/" + author.avatarMediaId : ""
        color: author.color
    }

    Item {
        Layout.preferredWidth: Kirigami.Units.gridUnit * 2
        Layout.preferredHeight: 1

        visible: !showAuthor
    }
    
    ColumnLayout {
        id: column
        Layout.fillWidth: true
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

            Kirigami.Avatar {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 1.5
                Layout.preferredHeight: Kirigami.Units.gridUnit * 1.5
                Layout.alignment: Qt.AlignTop

                source: replyVisible && reply.author.avatarMediaId ? "image://mxc/" + reply.author.avatarMediaId : ""
                name: replyVisible ? reply.author.displayName : "H"
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

                    text: replyVisible ? reply.display : ""

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
