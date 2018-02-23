import QtQuick 2.10
import QtQuick.Controls 2.3
import "qrc:/qml/form"

Page {
    ContactListForm {
        id: contactListForm
        height: parent.height
        width: 320
    }

    ContactDetailForm {
        id: contactDetailForm
        anchors.fill: parent
        anchors.leftMargin: contactListForm.width
    }
}
