import QtQuick 2.10
import QtQuick.Controls 2.3
import "qrc:/qml/form"

Page {
    property var contactListModel

    ListForm {
        id: roomListForm
        height: parent.height
        width: 320
        listModel: contactListModel
    }

    DetailForm {
        id: detailForm
        anchors.fill: parent
        anchors.leftMargin: roomListForm.width
    }
}
