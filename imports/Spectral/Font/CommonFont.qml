pragma Singleton
import QtQuick 2.9
import QtQuick.Controls 2.2

Item {
    property alias font: materialLabel.font
    Label {
        id: materialLabel
    }
}
