import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

AvatarContainer {
    Rectangle {
        id: messageRect

        width: messageImage.implicitWidth + 24
        height: messageImage.implicitHeight + 24

        color: sentByMe ? background : Material.accent

        DownloadableContent {
            id: downloadable

            width: messageImage.width
            height: messageImage.height
            anchors.centerIn: parent

            Image {
                id: messageImage
                z: -4
                sourceSize.width: 128
                source: "image://mxc/" + (content.thumbnail_url ? content.thumbnail_url : content.url)

                MouseArea {
                    anchors.fill: parent

                    hoverEnabled: true
                    propagateComposedEvents: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: content.body

                    onClicked: downloadable.downloadAndOpen()
                    onPressAndHold: messageImageMenu.popup()
                }

                Menu {
                    id: messageImageMenu

                    MenuItem {
                        text: "View"
                        onTriggered: downloadable.downloadAndOpen()
                    }
                    MenuItem {
                        text: "Save as..."
                        onTriggered: downloadable.saveFileAs()
                    }
                }
            }
        }
    }
}
