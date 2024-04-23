import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Page {
    id: callPage

    title: i18nc("@title", "Call")

    VideoOutput {
        id: video
        anchors.fill: parent
        visible: false
        Component.onCompleted: CallController.setVideoSink(video.videoSink)
    }

    VideoOutput {
        id: viewFinder
        anchors.centerIn: parent

        ToolBar {
            id: toolbar

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Kirigami.Units.gridUnit * 8

            z: 1000

            background: Kirigami.ShadowedRectangle {
                color: Kirigami.Theme.backgroundColor
                radius: 5

                shadow {
                    size: 15
                    yOffset: 3
                    color: Qt.rgba(0, 0, 0, 0.2)
                }

                border {
                    color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
                    width: 1
                }

                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.Window
            }
            RowLayout {
                ToolButton {
                    id: cameraButton
                    icon.name: "camera-on-symbolic"
                    text: i18nc("@action:button", "Enable Camera")
                    display: AbstractButton.IconOnly
                    checkable: true
                    onClicked: CallController.toggleCamera()

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }
        }
    }

    LivekitVideoSink {
        videoSink: viewFinder.videoSink
    }

    //Component.onCompleted: camera.start()

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
