import QtQuick 2.9
import QtQuick.Controls 2.2

ItemDelegate {
    text: category

    onClicked: {
        settingStackView.clear()
        settingStackView.push([accountForm, generalForm, appearanceForm, aboutForm][form])
    }
}
