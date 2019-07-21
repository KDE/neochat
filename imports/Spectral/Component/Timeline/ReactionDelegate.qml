import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Spectral.Setting 0.1

RowLayout {
    visible: reaction || false

    Repeater {
        model: reaction

        delegate: Control {
            horizontalPadding: 4
            verticalPadding: 0

            background: Rectangle {
                radius: height / 2
                color: MPalette.banner
            }

            contentItem: Label {
                text: modelData.reaction + " " + modelData.count
                font.pixelSize: 10
            }
        }
    }
}

