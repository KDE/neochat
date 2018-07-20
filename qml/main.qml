import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0 as Settings
import Qt.labs.platform 1.0 as Platform
import Matrique 0.1

import "component"
import "form"

ApplicationWindow {
    id: window
    visible: true
    width: 960
    height: 640
    minimumWidth: 480
    minimumHeight: 320
    title: qsTr("Matrique")

    Material.theme: settingPage.darkTheme ? Material.Dark : Material.Light

    FontLoader { id: materialFont; source: "qrc:/asset/font/material.ttf" }

    Settings.Settings {
        id: setting
        property alias homeserver: matriqueController.homeserver
        property alias userID: matriqueController.userID
        property alias token: matriqueController.token
    }

    Platform.SystemTrayIcon {
        id: trayIcon

        visible: true
        iconSource: "qrc:/asset/img/icon.png"

        onActivated: {
            window.show()
            window.raise()
            window.requestActivate()
        }
    }

    Controller {
        id: matriqueController
        onErrorOccured: {
            errorDialog.text = err;
            errorDialog.open();
        }
    }

    Popup {
        property bool busy: matriqueController.busy

        id: busyPopup

        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        focus: true

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        BusyIndicator { running: true }

        onBusyChanged: {
            if(busyPopup.busy) { busyPopup.open(); }
            else { busyPopup.close(); }
        }
    }

    Dialog {
        property alias text: errorLabel.text

        id: errorDialog
        width: 360
        modal: true
        title: "ERROR"

        x: (window.width - width) / 2
        y: (window.height - height) / 2

        standardButtons: Dialog.Ok

        Label {
            id: errorLabel
            width: parent.width
            text: "Label"
            wrapMode: Text.Wrap
        }
    }

    Component {
        id: loginPage

        Login { controller: matriqueController }
    }

    Room {
        id: roomPage
        connection: matriqueController.connection
    }

    Setting {
        id: settingPage
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SideNav {
            id: sideNav
            Layout.preferredWidth: 80
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                SideNavButton {
                    id: statusNavButton
                    contentItem: ImageStatus {
                        anchors.fill: parent
                        anchors.margins: 15

                        source: matriqueController.isLogin ? matriqueController.connection.localUser && matriqueController.connection.localUser.avatarUrl ? "image://mxc/" + matriqueController.connection.localUser.avatarUrl : "" : "qrc:/asset/img/avatar.png"
                        displayText: matriqueController.connection.localUser && matriqueController.connection.localUser.displayText ? matriqueController.connection.localUser.displayText : "N"
                        opaqueBackground: false
                    }

                    page: roomPage
                }

                Rectangle {
                    color: "transparent"
                    Layout.fillHeight: true
                }

                SideNavButton {
                    contentItem: MaterialIcon { icon: "\ue8b8"; color: "white" }
                    page: settingPage
                }

                SideNavButton {
                    contentItem: MaterialIcon { icon: "\ue879"; color: "white" }
                    onClicked: {
                        Qt.quit();
                    }
                }
            }
        }

        StackView {
            id: stackView
            initialItem: roomPage

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Component.onCompleted: {
        imageProvider.connection = matriqueController.connection

        if (matriqueController.userID && matriqueController.token) {
            matriqueController.login();
        } else {
            stackView.replace(loginPage);
        }
    }
}
