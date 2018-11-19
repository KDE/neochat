import QtQuick 2.9
import QtQuick.Controls 2.2

Label {
    text: section + " • " + Qt.formatTime(time, "hh:mm")
    font.pixelSize: 13
    font.weight: Font.Medium
    font.capitalization: Font.AllUppercase
    verticalAlignment: Text.AlignVCenter
}
