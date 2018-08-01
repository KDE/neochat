import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3

import "component"
import "form"

Page {
    property alias darkTheme: appearanceForm.darkTheme
    property alias miniMode: appearanceForm.miniMode
    property var connection

    SettingAccountForm {
        id: accountForm
        parent: null
    }

    SettingAppearanceForm {
        id: appearanceForm
        parent: null
    }

    RowLayout {
        ColumnLayout {
            Material.elevation: 10
            Layout.preferredWidth: 240
            Layout.fillHeight: true

            spacing: 0

            ItemDelegate {
                Layout.fillWidth: true

                text: "Account"
                onClicked: pushToStack(accountForm)
            }

            ItemDelegate {
                Layout.fillWidth: true

                text: "Appearance"
                onClicked: pushToStack(appearanceForm)
            }

            ItemDelegate {
                Layout.fillWidth: true

                text: "About"
            }
        }

        StackView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: settingStackView
        }
    }

    function pushToStack(item) {
        settingStackView.clear()
        settingStackView.push(item)
    }
}
