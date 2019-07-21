import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Spectral.Setting 0.1

RowLayout {
    visible: reaction || false

    Repeater {
        model: reaction

        delegate: Control {
            horizontalPadding: 6
            verticalPadding: 0

            background: Rectangle {
                radius: height / 2
                color: modelData.hasLocalUser ? (MSettings.darkTheme ? Qt.darker(MPalette.accent, 1.55) : Qt.lighter(MPalette.accent, 1.55)) : MPalette.banner
            }

            contentItem: Label {
                text: modelData.reaction + " " + modelData.count
                font.pixelSize: 14
            }
        }
    }
}

