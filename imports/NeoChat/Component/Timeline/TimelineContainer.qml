/**
 * SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.4 as Kirigami

Item {
    default property alias innerObject : column.children

    height: column.implicitHeight + (readMarker ? 2 * Kirigami.Units.smallSpacing : 0)

    ColumnLayout {
        id: column
        width: parent.width

        SectionDelegate {
            Layout.maximumWidth: parent.width
            Layout.alignment: Qt.AlignHCenter

            visible: showSection
        }
    }

    Rectangle {
        width: parent.width * 0.9
        x: parent.width * 0.05
        height: Kirigami.Units.smallSpacing
        anchors.top: column.bottom
        anchors.topMargin: Kirigami.Units.smallSpacing
        visible: readMarker
        color: Kirigami.Theme.positiveTextColor
    }
}
