import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.3

Drawer {
    property SwipeView swipeView

    interactive: false
    position: 1.0
    visible: true
    modal: false

    background: Rectangle {
        color: Material.accent
    }
}
