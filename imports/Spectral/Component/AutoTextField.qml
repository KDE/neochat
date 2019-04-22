import QtQuick 2.12
import QtQuick.Controls 2.12

import QtQuick.Controls.Material 2.3

TextField {
    id: textField

    selectByMouse: true

    topPadding: 8
    bottomPadding: 8

    background: Item {
        Label {
            id: floatingPlaceholder

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: textField.topPadding
            anchors.leftMargin: textField.leftPadding
            transformOrigin: Item.TopLeft
            visible: false
            color: Material.accent

            states: [
                State {
                    name: "shown"
                    when: textField.text.length !== 0
                    PropertyChanges { target: floatingPlaceholder; scale: 0.8 }
                    PropertyChanges { target: floatingPlaceholder; anchors.topMargin: -floatingPlaceholder.height * 0.4 }
                }
            ]

            transitions: [
                Transition {
                    to: "shown"
                    SequentialAnimation {
                        PropertyAction { target: floatingPlaceholder; property: "text"; value: textField.placeholderText }
                        PropertyAction { target: floatingPlaceholder; property: "visible"; value: true }
                        PropertyAction { target: textField; property: "placeholderTextColor"; value: "transparent" }
                        ParallelAnimation {
                            NumberAnimation { target: floatingPlaceholder; property: "scale"; duration: 250; easing.type: Easing.InOutQuad }
                            NumberAnimation { target: floatingPlaceholder; property: "anchors.topMargin"; duration: 250; easing.type: Easing.InOutQuad }
                        }
                    }
                },
                Transition {
                    from: "shown"
                    SequentialAnimation {
                        ParallelAnimation {
                            NumberAnimation { target: floatingPlaceholder; property: "scale"; duration: 250; easing.type: Easing.InOutQuad }
                            NumberAnimation { target: floatingPlaceholder; property: "anchors.topMargin"; duration: 250; easing.type: Easing.InOutQuad }
                        }
                        PropertyAction { target: textField; property: "placeholderTextColor"; value: "grey" }
                        PropertyAction { target: floatingPlaceholder; property: "visible"; value: false }
                    }
                }
            ]
        }
    }
}
