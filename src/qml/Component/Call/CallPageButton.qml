// SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.0
import QtQuick.Controls 2.7 as QQC2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.13 as Kirigami

QQC2.AbstractButton {
    id: control

    property int temprament: CallPageButton.Neutral
    property bool shimmering: false

    enum Temprament {
        Neutral,
        Constructive,
        Destructive
    }

    padding: Kirigami.Units.largeSpacing
    contentItem: ColumnLayout {
        QQC2.Control {
            padding: Kirigami.Units.gridUnit

            Kirigami.Theme.colorSet: Kirigami.Theme.Button
            Layout.alignment: Qt.AlignHCenter

            contentItem: Kirigami.Icon {
                implicitHeight: Kirigami.Units.iconSizes.medium
                implicitWidth: Kirigami.Units.iconSizes.medium
                source: control.icon.name
            }
            background: Rectangle {
                Kirigami.Theme.colorSet: Kirigami.Theme.Button

                ShimmerGradient {
                    id: shimmerGradient
                    color: {
                        switch (control.temprament) {
                        case CallPageButton.Neutral:
                            return Kirigami.Theme.textColor
                        case CallPageButton.Constructive:
                            return Kirigami.Theme.positiveTextColor
                        case CallPageButton.Destructive:
                            return Kirigami.Theme.negativeTextColor
                        }
                    }
                }

                color: {
                    if (control.checked) {
                        return Kirigami.Theme.focusColor
                    }

                    switch (control.temprament) {
                    case CallPageButton.Neutral:
                        return Kirigami.Theme.backgroundColor
                    case CallPageButton.Constructive:
                        return Kirigami.Theme.positiveBackgroundColor
                    case CallPageButton.Destructive:
                        return Kirigami.Theme.negativeBackgroundColor
                    }
                }
                border.color: Kirigami.Theme.focusColor
                border.width: control.visualFocus ? 2 : 0
                radius: height/2

                Rectangle {
                    visible: control.shimmering
                    anchors.fill: parent
                    radius: height/2

                    gradient: control.shimmering ? shimmerGradient : null
                }
            }
        }
        QQC2.Label {
            text: control.text
            font: Kirigami.Theme.smallFont

            horizontalAlignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }
    }
}
