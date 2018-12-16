import QtQuick 2.12
import QtQuick.Controls 2.4
import Spectral.Setting 0.1

Label {
    text: section + " • " + Qt.formatTime(time, "hh:mm")
    color: MPalette.foreground
    font.pixelSize: 13
    font.weight: Font.Medium
    font.capitalization: Font.AllUppercase
    verticalAlignment: Text.AlignVCenter
}
