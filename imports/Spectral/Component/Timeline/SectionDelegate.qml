import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls

import org.kde.kirigami 2.4 as Kirigami

Controls.Label {
    text: section + " • " + Qt.formatTime(time)
    font.weight: Font.Medium
    font.capitalization: Font.AllUppercase
    verticalAlignment: Text.AlignVCenter
    padding: 8
}
