// SPDX-FileCopyrightText: 2021 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Particles

import org.kde.kirigami as Kirigami

Item {
    id: root
    property bool enabled: false
    property int effectInterval: Kirigami.Units.veryLongDuration*10;
    property color darkSnowColor: "grey"
    property bool isThemeDark: Kirigami.Theme.backgroundColor.hslLightness <= darkSnowColor.hslLightness

    function showConfettiEffect() {
        confettiTimer.start()
    }

    function showSnowEffect() {
        snowTimer.start()
    }

    function showFireworksEffect() {
        fireworksTimer.start()
    }

    // Confetti

    Timer {
        id: confettiTimer
        interval: root.effectInterval;
        running: false;
        repeat: false;
        triggeredOnStart: true;
        onTriggered: {
            if (root.enabled) {
                confettiSystem.running = !confettiSystem.running
            }
        }
    }

    ParticleSystem {
        id: confettiSystem
        anchors.fill: parent

        running: false
        onRunningChanged: {
            if (running) {
                opacity = 1
            } else {
                opacity = 0
            }
        }

        Behavior on opacity {
            SequentialAnimation {
                NumberAnimation { duration: Kirigami.Units.longDuration }
            }
        }

        ImageParticle {
            source: "qrc:/confetti.png"
            entryEffect: ImageParticle.Scale
            rotationVariation: 360
            rotationVelocity: 90
            color: Qt.hsla(Math.random(), 0.5, 0.6, 1)
            colorVariation: 1
        }

        Emitter {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }

            sizeVariation: Kirigami.Units.iconSizes.small/2
            lifeSpan: Kirigami.Units.veryLongDuration*10
            size: Kirigami.Units.iconSizes.small

            velocity: AngleDirection {
                angle: 90
                angleVariation: 42
                magnitude: 500
            }
        }
    }

    // Snow

    Timer {
        id: snowTimer
        interval: root.effectInterval;
        running: false;
        repeat: false;
        triggeredOnStart: true;
        onTriggered: {
            if (root.enabled) {
                snowSystem.running = !snowSystem.running
            }
        }
    }

    ParticleSystem {
        id: snowSystem
        anchors.fill: parent

        running: false
        onRunningChanged: {
            if (running) {
                opacity = 1
            } else {
                opacity = 0
            }
        }

        Behavior on opacity {
            SequentialAnimation {
                NumberAnimation { duration: Kirigami.Units.longDuration }
            }
        }

        ItemParticle {
            delegate: Rectangle {
                width: 10
                height: width
                radius: width
                color: root.isThemeDark ? "white" : darkSnowColor
                scale: Math.random()
                opacity: Math.random()
            }
        }

        Emitter {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }

            sizeVariation: Kirigami.Units.iconSizes.medium
            lifeSpan: Kirigami.Units.veryLongDuration*10
            size: Kirigami.Units.iconSizes.large
            emitRate: 42

            velocity: AngleDirection {
                angle: 90
                angleVariation: 10
                magnitude: 300
            }
        }
    }

    // Fireworks

    Timer {
        id: fireworksTimer
        interval: root.effectInterval;
        running: false;
        repeat: false;
        triggeredOnStart: true;
        onTriggered: {
            if (root.enabled) {
                fireworksInternalTimer.running = !fireworksInternalTimer.running
            }
        }
    }

    Timer {
        id: fireworksInternalTimer
        interval: 300
        triggeredOnStart: true
        running: false
        repeat: true
        onTriggered: {
            var x = Math.random() * parent.width
            var y = Math.random() * parent.height
            customEmit(x, y)
            customEmit(x, y)
            customEmit(x, y)
        }
    }

    ParticleSystem {
        id: fireworksSystem
        anchors.fill: parent
        running: fireworksInternalTimer.running
        onRunningChanged: {
            if (running) {
                opacity = 1
            } else {
                opacity = 0
            }
        }

        Behavior on opacity {
            SequentialAnimation {
                NumberAnimation { duration: Kirigami.Units.longDuration }
            }
        }
    }

    ImageParticle {
        id: fireworksParticleA
        system: fireworksSystem
        source: "qrc:/glowdot.png"
        alphaVariation: root.isThemeDark ? 0.1 : 0.1
        alpha: root.isThemeDark ? 0.5 : 1
        groups: ["a"]
        opacity: fireworksSystem.opacity
        entryEffect: ImageParticle.Scale
        rotationVariation: 360
    }

    ImageParticle {
        system: fireworksSystem
        source: "qrc:/glowdot.png"
        color: root.isThemeDark ? "white" : "gold"
        alphaVariation: root.isThemeDark ? 0.1 : 0.1
        alpha: root.isThemeDark ? 0.5 : 1
        groups: ["light"]
        opacity: fireworksSystem.opacity
        entryEffect: ImageParticle.Scale
        rotationVariation: 360
    }

    ImageParticle {
        id: fireworksParticleB
        system: fireworksSystem
        source: "qrc:/glowdot.png"
        alphaVariation: root.isThemeDark ? 0.1 : 0.1
        alpha: root.isThemeDark ? 0.5 : 1
        groups: ["b"]
        opacity: fireworksSystem.opacity
        entryEffect: ImageParticle.Scale
        rotationVariation: 360
    }

    Component {
        id: emitterComp
        Emitter {
            id: container
            property int life: 23
            property real targetX: 0
            property real targetY: 0
            width: 1
            height: 1
            system: fireworksSystem
            size: 16
            endSize: 8
            sizeVariation: 5
            Timer {
                interval: life
                running: true
                onTriggered: {
                    container.destroy();
                    var randomHue = Math.random()
                    var lightness = root.isThemeDark ? 0.8 : 0.7
                    fireworksParticleA.color = Qt.hsla(randomHue, 0.8, lightness, 1)
                    fireworksParticleB.color = Qt.hsla(1-randomHue, 0.8, lightness, 1)
                }
            }
            velocity: AngleDirection {angleVariation:360; magnitude: 200}
        }
    }

    function customEmit(x,y) {
        var currentSize = Math.round(Math.random() * 200) + 40
        var currentLifeSpan = Math.round(Math.random() * 1000) + 100
        for (var i=0; i<8; i++) {
            var obj = emitterComp.createObject(parent);
            obj.x = x
            obj.y = y
            obj.targetX = Math.random() * currentSize - currentSize/2 + obj.x
            obj.targetY = Math.random() * currentSize - currentSize/2 + obj.y
            obj.life = Math.round(Math.random() * 23) + 150
            obj.emitRate = Math.round(Math.random() * 32) + 5
            obj.lifeSpan = currentLifeSpan
            const group = Math.round(Math.random() * 3);
            switch (group) {
                case 0:
                    obj.group = "light";
                    break;
                case 1:
                    obj.group = "a";
                    break;
                case 2:
                    obj.group = "b";
                    break;
            }
        }
    }
}
