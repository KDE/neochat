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

    height: column.implicitHeight

    ColumnLayout {
        id: column
        width: parent.width

        SectionDelegate {
            Layout.maximumWidth: parent.width
            Layout.alignment: Qt.AlignHCenter

            visible: showSection
        }
    }
}
