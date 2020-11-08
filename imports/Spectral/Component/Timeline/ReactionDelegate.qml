/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

Flow {
    visible: (reaction && reaction.length > 0) ?? false

    spacing: 8

    Repeater {
        model: reaction

        delegate: Control {
            width: Math.min(implicitWidth, 128)
            height: width

            horizontalPadding: 6
            verticalPadding: 0

            contentItem: Label {
                text: modelData.reaction + (modelData.count > 1 ? " " + modelData.count : "")
                font.pixelSize: 14
                font.family: "emoji"
                elide: Text.ElideRight
            }

            background: Rectangle {
                radius: height / 2
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                color: Kirigami.Theme.backgroundColor

                MouseArea {
                    anchors.fill: parent

                    hoverEnabled: true

                    ToolTip.visible: containsMouse
                    TootTip.font.family: Kirigami.Theme.defaultFont.family + ", emoji"
                    ToolTip.text: {
                        var text = "";

                        for (var i = 0; i < modelData.authors.length; i++) {
                            if (i === modelData.authors.length - 1 && i !== 0) {
                                text += i18nc("Seperate the usernames of users", " and ")
                            } else if (i !== 0) {
                                text += ", "
                            }

                            text += modelData.authors[i].displayName
                        }

                        text = i18ncp("%1 is the users who reacted and %2 the emoji that was given", "%2 reacted with %3", "%2 reacted with %3", modelData.authors.length, text, modelData.reaction)

                        return text
                    }

                    onClicked: currentRoom.toggleReaction(eventId, modelData.reaction)
                }
            }
        }
    }
}

