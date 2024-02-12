// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.neochat.config

Flow {
    id: root

    /**
     * @brief The reaction model to get the reactions from.
     */
    property alias model: reactionRepeater.model

    /**
     * @brief The given reaction has been clicked.
     *
     * Thrown when one of the reaction buttons in the flow is clicked.
     */
    signal reactionClicked(string reaction)

    spacing: Kirigami.Units.smallSpacing

    Repeater {
        id: reactionRepeater

        delegate: QQC2.AbstractButton {
            id: reactionDelegate

            required property string textContent
            required property string reaction
            required property string toolTip
            required property bool hasLocalUser

            width: Math.max(contentItem.implicitWidth + leftPadding + rightPadding, height)
            height: Math.round(Kirigami.Units.gridUnit * 1.5)

            contentItem: Item {
                QQC2.Label {
                    id: reactionLabel
                    anchors.centerIn: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: reactionDelegate.textContent
                    background: null
                    wrapMode: TextEdit.NoWrap
                    textFormat: Text.RichText
                }
            }

            padding: Kirigami.Units.smallSpacing

            background: Kirigami.ShadowedRectangle {
                color: reactionDelegate.hasLocalUser ? Kirigami.Theme.positiveBackgroundColor : Kirigami.Theme.backgroundColor
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Config.compactLayout ? Kirigami.Theme.Window : Kirigami.Theme.View
                radius: height / 2
                shadow {
                    size: Kirigami.Units.smallSpacing
                    color: !reactionDelegate.hasLocalUser ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                }
            }

            onClicked: reactionClicked(reactionDelegate.reaction)

            hoverEnabled: true

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: reactionDelegate.toolTip
        }
    }
}

