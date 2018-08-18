import QtQuick 2.9
import QtQuick.Controls 2.2

Item {
    property alias source: baseImage.source
    property alias sourceSize: baseImage.sourceSize.width

    readonly property bool loading: baseImage.status == Image.Loading
    signal clicked()

    id: rekt

    width: loading ? 128 : baseImage.implicitWidth
    height: loading ? progressBar.height : baseImage.implicitHeight

    Image {
        id: baseImage
    }

    ProgressBar {
        id: progressBar
        width: parent.width
        visible: loading

        indeterminate: true
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true

        onClicked: rekt.clicked()
    }
}
