/* SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-FileCopyrightText: 2021 Srevin Saju <srevinsaju@sugarlabs.org>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.14 as Kirigami
import org.kde.neochat 1.0

Loader {
    id: root
    property var typingNotification: null

    active: visible
    sourceComponent: Pane {
        id: typingPane

        padding: fontMetrics.lineSpacing * 0.25
        spacing: 0
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        contentItem: RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft
            spacing: 0

            FontMetrics {
                id: fontMetrics
                font: typingLabel.font
            }
            Label {
                id: typingLabel
                textFormat: TextEdit.RichText
                wrapMode: Label.Wrap
                text: typingNotification
            }
            BusyIndicator {
                running: root.active
                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
            }
        }

        background: Item {
            Rectangle {
                height: 1
                property color borderColor: Kirigami.Theme.textColor
                color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, 0.1)
                anchors {
                    left: typingIndicatorBackground.left
                    right: typingIndicatorBackground.right
                    bottom: typingIndicatorBackground.top
                }
            }
            Rectangle {
                anchors.fill: parent
                id: typingIndicatorBackground
                color: Kirigami.Theme.backgroundColor
            }
        }
    }
}
