import QtQuick 2.12
import QtQuick.Controls 2.4

Label {
    text: section + " • " + Qt.formatTime(time, "hh:mm")
    font.pixelSize: 13
    font.weight: Font.Medium
    font.capitalization: Font.AllUppercase
    verticalAlignment: Text.AlignVCenter
}
