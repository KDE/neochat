pragma Singleton
import QtQuick 2.12
import QtQuick.Controls 2.4

Item {
    property alias font: materialLabel.font
    Label {
        id: materialLabel
    }
}
