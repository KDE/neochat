import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Spectral.Settings 0.1

Text {
    property alias icon: materialLabel.text

    id: materialLabel

    font.pointSize: 16
    font.family: materialFont.name
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
}
