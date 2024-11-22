import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Page {
    id: callPage

    title: i18nc("@title", "Call")

    RowLayout {
        anchors.fill: parent

        VideoOutput {
            id: viewFinder

            Layout.fillWidth: parent.width / 2
            Layout.preferredHeight: parent.height

            Label {
                text: "You"
            }
        }

        VideoOutput {
            id: otherViewFinder

            Layout.fillWidth: parent.width / 2
            Layout.preferredHeight: parent.height

            Label {
                text: "Them"
            }
        }
    }

    LivekitVideoSink {
        videoSink: otherViewFinder.videoSink
    }

    Component.onCompleted: camera.start()

    Connections {
        target: CallController

        function onConnected(): void {
            CallController.setCameraVideoSink(viewFinder.videoSink)
            CallController.toggleCamera()
        }
    }

    CaptureSession {
        camera: Camera {
            id: camera
        }
        imageCapture: ImageCapture {
            id: imageCapture
        }
        videoOutput: viewFinder
    }
}
