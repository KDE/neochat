import QtQuick 2.12
import QtQuick.Controls 2.12
import Spectral.Setting 0.1

Label {
    text: section + " • " + Qt.formatTime(time)
    color: MPalette.foreground
    font.pixelSize: 13
    font.weight: Font.Medium
    font.capitalization: Font.AllUppercase
    verticalAlignment: Text.AlignVCenter
    padding: 8
}
