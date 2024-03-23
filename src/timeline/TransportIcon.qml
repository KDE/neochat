/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

/** Displays a transport line or mode logo/icon.
 *  Mainly to hide ugly implementation details of Icon not
 *  handling non-square SVG assets in the way we need it here.
 */
Item {
    id: root
    // properties match those of Icon
    property string source
    property alias isMask: __icon.isMask
    property alias color: __icon.color

    // icon size (height for non-square ones)
    property int size: Kirigami.Units.iconSizes.small

    property bool __isIcon: !source.startsWith("file:")


    implicitWidth: __isIcon ? root.size : Math.round(root.size * __image.implicitWidth / __image.implicitHeight)
    implicitHeight: root.size

    Layout.preferredWidth: root.implicitWidth
    Layout.preferredHeight: root.implicitHeight

    Kirigami.Icon {
        id: __icon
        source: root.__isIcon ? root.source : ""
        visible: source !== ""
        anchors.fill: parent
    }
    Image {
        id: __image
        source: root.__isIcon ? "" : root.source
        visible: source !== ""
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
    }
}
