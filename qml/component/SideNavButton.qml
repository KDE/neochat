import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

ItemDelegate {
    property var page
    readonly property bool selected: stackView.currentItem === page

    Rectangle {
        width: selected ? 4 : 0
        height: parent.height

        color: Material.accent

        Behavior on width {
            PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
        }
    }

    onClicked: {
        if(page && stackView.currentItem !== page) {
            if(stackView.depth === 1) {
                stackView.replace(page)
            } else {
                stackView.clear()
                stackView.push(page)
            }
        }
    }
}
