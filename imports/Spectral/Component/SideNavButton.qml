import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import "qrc:/js/util.js" as Util

ItemDelegate {
    property var page
    property bool selected: stackView.currentItem === page
    property color highlightColor: Material.accent

    Rectangle {
        width: selected ? 4 : 0
        height: parent.height

        color: highlightColor

        Behavior on width {
            PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
        }
    }

    onClicked: Util.pushToStack(stackView, page)
}
