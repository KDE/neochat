import QtQuick 2.9
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

import Spectral.Component 2.0
import Spectral.Setting 0.1

AutoMouseArea {
    id: ripple

    property color color: MSettings.darkTheme ? Qt.rgba(255, 255, 255, 0.16) : Qt.rgba(0, 0, 0, 0.08)
    property bool circular: false
    property bool centered: false
    property bool focused
    property color focusColor: "transparent"
    property int focusWidth: width - 32
    property Item control

    clip: true

    Connections {
        target: control

        onPressedChanged: {
            if (!control.pressed)
                __private.removeLastCircle()
        }
    }

    onPressed: {
        __private.createTapCircle(mouse.x, mouse.y)

        if (control)
            mouse.accepted = false
    }

    onReleased: __private.removeLastCircle()
    onCanceled: __private.removeLastCircle()

    QtObject {
        id: __private

        property int startRadius: circular ? width/10 : width/6
        property int endRadius
        property bool showFocus: true

        property Item lastCircle

        function createTapCircle(x, y) {
            endRadius = centered ? width/2 : radius(x, y) + 5
            showFocus = false

            lastCircle = tapCircle.createObject(ripple, {
                "circleX": centered ? width/2 : x,
                "circleY": centered ? height/2 : y
            })
        }

        function removeLastCircle() {
            if (lastCircle)
                lastCircle.removeCircle()
        }

        function radius(x, y) {
            var dist1 = Math.max(dist(x, y, 0, 0), dist(x, y, width, height))
            var dist2 = Math.max(dist(x, y, width, 0), dist(x, y, 0, height))

            return Math.max(dist1, dist2)
        }

        function dist(x1, y1, x2, y2) {
            var distX = x2 - x1
            var distY = y2 - y1

            return Math.sqrt(distX * distX + distY * distY)
        }
    }

    Rectangle {
        id: focusBackground
        objectName: "focusBackground"

        width: parent.width
        height: parent.height

        color: Qt.rgba(0,0,0,0.2)

        opacity: __private.showFocus && focused ? 1 : 0

        Behavior on opacity {
            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
        }
    }

    Rectangle {
        id: focusCircle
        objectName: "focusRipple"

        property bool focusedState

        x: (parent.width - width)/2
        y: (parent.height - height)/2

        width: focused
                ? focusedState ? focusWidth
                               : Math.min(parent.width - 8, focusWidth + 12)
                : parent.width/5
        height: width

        radius: width/2

        opacity: __private.showFocus && focused ? 1 : 0

        color: focusColor.a === 0 ? Qt.rgba(1,1,1,0.4) : focusColor

        Behavior on opacity {
            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
        }

        Behavior on width {
            NumberAnimation { duration: focusTimer.interval; }
        }

        Timer {
            id: focusTimer
            running: focused
            repeat: true
            interval: 800

            onTriggered: focusCircle.focusedState = !focusCircle.focusedState
        }
    }

    Component {
        id: tapCircle

        Item {
            id: circleItem
            objectName: "tapRipple"

            property bool done

            property real circleX
            property real circleY

            property bool closed

            width: parent.width
            height: parent.height

            function removeCircle() {
                done = true

                if (fillSizeAnimation.running) {
                    fillOpacityAnimation.stop()
                    closeAnimation.start()

                    circleItem.destroy(500);
                } else {
                    __private.showFocus = true
                    fadeAnimation.start();

                    circleItem.destroy(300);
                }
            }

            Item {
                id: circleParent

                width: parent.width
                height: parent.height

                visible: !circular

                Rectangle {
                    id: circleRectangle

                    x: circleItem.circleX - radius
                    y: circleItem.circleY - radius

                    width: radius * 2
                    height: radius * 2

                    opacity: 0
                    color: ripple.color

                    NumberAnimation {
                        id: fillSizeAnimation
                        running: true

                        target: circleRectangle; property: "radius"; duration: 500;
                        from: __private.startRadius; to: __private.endRadius;
                        easing.type: Easing.InOutQuad

                        onStopped: {
                            if (done)
                                __private.showFocus = true
                        }
                    }

                    NumberAnimation {
                        id: fillOpacityAnimation
                        running: true

                        target: circleRectangle; property: "opacity"; duration: 300;
                        from: 0; to: 1; easing.type: Easing.InOutQuad
                    }

                    NumberAnimation {
                        id: fadeAnimation

                        target: circleRectangle; property: "opacity"; duration: 300;
                        from: 1; to: 0; easing.type: Easing.InOutQuad
                    }

                    SequentialAnimation {
                        id: closeAnimation

                        NumberAnimation {
                            target: circleRectangle; property: "opacity"; duration: 250;
                            to: 1; easing.type: Easing.InOutQuad
                        }

                        NumberAnimation {
                            target: circleRectangle; property: "opacity"; duration: 250;
                            from: 1; to: 0; easing.type: Easing.InOutQuad
                        }
                    }
                }
            }

            CircleMask {
                anchors.fill: parent
                source: circleParent
                visible: circular
            }
        }
    }
}
