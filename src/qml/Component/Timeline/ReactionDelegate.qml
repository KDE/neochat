// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

Flow {
    spacing: Kirigami.Units.smallSpacing

    Repeater {
        model: reaction ?? null

        delegate: QQC2.AbstractButton {
            width: Math.max(implicitWidth, height)

            contentItem: QQC2.Label {
                horizontalAlignment: Text.AlignHCenter
                text: modelData.reaction + " " + modelData.count
            }

            padding: Kirigami.Units.smallSpacing

            background: Kirigami.ShadowedRectangle {
                color: checked ? Kirigami.Theme.positiveBackgroundColor : Kirigami.Theme.backgroundColor
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                radius: height / 2
                shadow.size: Kirigami.Units.smallSpacing
                shadow.color: !model.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                border.width: 1
            }

            checkable: true

            checked: modelData.hasLocalUser

            onToggled: currentRoom.toggleReaction(eventId, modelData.reaction)

            hoverEnabled: true

            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: {
                var text = "";

                for (var i = 0; i < modelData.authors.length && i < 3; i++) {
                    if (i !== 0) {
                        if (i < modelData.authors.length - 1) {
                            text += ", "
                        } else {
                            text += i18nc("Separate the usernames of users", " and ")
                        }
                    }
                    text += currentRoom.htmlSafeMemberName(modelData.authors[i].id)
                }
                if (modelData.authors.length > 3) {
                    text += i18ncp("%1 is the number of other users", " and %1 other", " and %1 others", modelData.authors.length - 3)
                }

                text = i18ncp("%2 is the users who reacted and %3 the emoji that was given", "%2 reacted with %3", "%2 reacted with %3", modelData.authors.length, text, modelData.reaction)

                return text
            }
        }
    }
}

