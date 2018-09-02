import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3
import Matrique.Settings 0.1

Rectangle {
    property bool flat: false
    property bool highlighted: false
    property string displayText: ""
    property alias timeLabelVisible: timeText.visible
    property alias authorLabelVisible: authorText.visible

    property int maximumWidth

    readonly property bool darkBackground: highlighted && !flat

    id: messageRect

    width: Math.min(Math.max(messageText.implicitWidth, (timeText.visible ? timeText.implicitWidth : 0), (authorLabelVisible ? authorText.implicitWidth : 0)) + 24, maximumWidth)
    height: (authorText.visible ? authorText.implicitHeight : 0) + messageText.implicitHeight + (timeText.visible ? timeText.implicitHeight : 0) + 24

    color: flat ? "transparent" : highlighted ? Material.primary : background
    border.color: Material.primary
    border.width: flat ? 2 : 0

    ColumnLayout {
        id: messageColumn

        anchors.fill: parent
        anchors.margins: 12
        spacing: 0

        Label {
            id: authorText
            text: author.displayName
            color: darkBackground ? "white" : Material.accent
            font.bold: true
        }

        Label {
            id: messageText
            Layout.maximumWidth: parent.width
            text: displayText
            color: darkBackground ? "white": Material.foreground

            wrapMode: Label.Wrap
            linkColor: darkBackground ? "white" : Material.accent
            textFormat: MSettings.richText ? Text.RichText : Text.StyledText
            onLinkActivated: Qt.openUrlExternally(link)
        }

        Label {
            id: timeText
            Layout.alignment: Qt.AlignRight
            text: Qt.formatTime(time, "hh:mm")
            color: darkBackground ? "white" : "grey"
            font.pointSize: 8
        }
    }
}
