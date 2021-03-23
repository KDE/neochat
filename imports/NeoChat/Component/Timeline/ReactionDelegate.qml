/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

Flow {
    spacing: Kirigami.Units.largeSpacing
    Repeater {
        model: reaction ?? null

        delegate: AbstractButton {
            width: Math.max(implicitWidth, height)

            contentItem: Label {
                horizontalAlignment: Text.AlignHCenter
                text: modelData.reaction + " " + modelData.count
            }

            padding: Kirigami.Units.smallSpacing

            background: Kirigami.ShadowedRectangle {
                color: checked ? Kirigami.Theme.positiveBackgroundColor : Kirigami.Theme.backgroundColor
                radius: height / 2
                shadow.size: Kirigami.Units.smallSpacing
                shadow.color: !model.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                border.width: Kirigami.Units.devicePixelRatio
            }


            checkable: true

            checked: modelData.hasLocalUser

            onToggled: currentRoom.toggleReaction(eventId, modelData.reaction)

            hoverEnabled: true

            ToolTip.visible: hovered
            ToolTip.text: {
                var text = "";

                for (var i = 0; i < modelData.authors.length; i++) {
                    if (i === modelData.authors.length - 1 && i !== 0) {
                        text += i18nc("Separate the usernames of users", " and ")
                    } else if (i !== 0) {
                        text += ", "
                    }

                    text += modelData.authors[i].displayName
                }

                text = i18ncp("%1 is the users who reacted and %2 the emoji that was given", "%2 reacted with %3", "%2 reacted with %3", modelData.authors.length, text, modelData.reaction)

                return text
            }
        }
    }
}

