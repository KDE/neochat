// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

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
        model: root.model

        delegate: QQC2.Control {
            width: Math.max(reactionTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 4, height)

            contentItem: TextEdit {
                id: reactionLabel
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: model.text
                readOnly: true
                Component.onCompleted: EmojiFixer.addTextDocument(reactionLabel.textDocument)
                color: Kirigami.Theme.textColor
                selectByMouse: false
                TextMetrics {
                    id: reactionTextMetrics
                    text: reactionLabel.text
                }
            }

            padding: Kirigami.Units.smallSpacing

            background: Kirigami.ShadowedRectangle {
                color: model.hasLocalUser ? Kirigami.Theme.positiveBackgroundColor : Kirigami.Theme.backgroundColor
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                radius: height / 2
                shadow.size: Kirigami.Units.smallSpacing
                shadow.color: !model.hasLocalUser ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                border.width: 1
            }

            MouseArea {
                anchors.fill: parent
                onClicked: reactionClicked(model.reaction)
                hoverEnabled: true
            }


            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: model.toolTip
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }
}

