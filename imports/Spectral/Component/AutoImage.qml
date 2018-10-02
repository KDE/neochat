import QtQuick 2.9
import QtQuick.Controls 2.2

Item {
    property alias source: baseImage.source
    property alias sourceSize: baseImage.sourceSize.width

    readonly property bool loading: baseImage.status == Image.Loading

    signal clicked()

    width: loading ? 128 : baseImage.implicitWidth
    height: loading ? progressBar.height : baseImage.implicitHeight

    id: rekt

    Image { id: baseImage }

    ProgressBar {
        width: parent.width
        visible: loading

        id: progressBar

        indeterminate: true
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true

        onClicked: rekt.clicked()
    }
}
