import QtQuick 2.9
import QtQuick.Controls 2.2

Label {
    text: section + " • " + Qt.formatTime(time, "hh:mm")
    color: "#1D333E"
    font.pointSize: 9.75
    font.weight: Font.Medium
    font.capitalization: Font.AllUppercase
    verticalAlignment: Text.AlignVCenter
}
