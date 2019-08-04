import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Spectral.Setting 0.1

Flow {
    visible: (reaction && reaction.length > 0) || false

    spacing: 8

    Repeater {
        model: reaction

        delegate: Control {
            horizontalPadding: 6
            verticalPadding: 0

            background: Rectangle {
                radius: height / 2
                color: modelData.hasLocalUser ? (MSettings.darkTheme ? Qt.darker(MPalette.accent, 1.55) : Qt.lighter(MPalette.accent, 1.55)) : MPalette.banner

                MouseArea {
                    anchors.fill: parent

                    hoverEnabled: true

                    ToolTip.visible: containsMouse
                    ToolTip.text: {
                        var text = "";

                        for (var i = 0; i < modelData.authors.length; i++) {
                            if (i === modelData.authors.length - 1 && i !== 0) {
                                text += " and "
                            } else if (i !== 0) {
                                text += ", "
                            }

                            text += modelData.authors[i].displayName
                        }

                        text += " reacted with " + modelData.reaction

                        return text
                    }

                    onClicked: currentRoom.toggleReaction(eventId, modelData.reaction)
                }
            }

            contentItem: Label {
                text: modelData.reaction + " " + modelData.count
                font.pixelSize: 14
            }
        }
    }
}

