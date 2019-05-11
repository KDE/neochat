import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import Spectral.Component 2.0
import Spectral.Setting 0.1

Control {
    padding: 8

    contentItem: RowLayout {
        Avatar {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24

            hint: author.displayName
            source: author.avatarMediaId
        }

        Label {
            Layout.fillWidth: true
            Layout.maximumWidth: messageListView.width - 48

            text: "<b>" + author.displayName + "</b> " + display + " • " + Qt.formatTime(time, "hh:mm AP")
            color: MPalette.foreground
            font.pixelSize: 13
            font.weight: Font.Medium
            textFormat: Label.StyledText

            wrapMode: Label.Wrap
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
