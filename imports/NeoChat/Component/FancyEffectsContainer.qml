/**
 * SPDX-FileCopyrightText: 2021 Alexey Andreyev <aa13q@ya.ru>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12
import QtQuick.Particles 2.12

import org.kde.kirigami 2.4 as Kirigami

Item {
    id: item
    property bool enabled: false
    property int effectInterval: Kirigami.Units.veryLongDuration*10;

    function showConfettiEffect() {
        confettiTimer.start()
    }

    function showSnowEffect() {
        snowTimer.start()
    }

    function showFireworkEffect() {

    }

    Timer {
        id: confettiTimer
        interval: item.effectInterval;
        running: false;
        repeat: false;
        triggeredOnStart: true;
        onTriggered: {
            if (item.enabled) {
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

        ItemParticle {
            delegate: Rectangle {
                width: Kirigami.Units.iconSizes.small
                height: Kirigami.Units.iconSizes.smallMedium
                color: Qt.hsla(Math.random(), 0.5, 0.6, 1)
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

            velocity: AngleDirection {
                angle: 90
                angleVariation: 42
                magnitude: 500
            }
        }
    }

    Timer {
        id: snowTimer
        interval: item.effectInterval;
        running: false;
        repeat: false;
        triggeredOnStart: true;
        onTriggered: {
            if (item.enabled) {
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
                color: "white"
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
}
