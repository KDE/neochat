import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import Spectral.Setting 0.1
import Spectral.Font 0.1

Text {
    property alias icon: materialLabel.text

    id: materialLabel

    color: MPalette.foreground
    font.pixelSize: 24
    font.family: MaterialFont.name
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
}
