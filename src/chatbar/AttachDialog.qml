// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.Popup {
    id: root

    padding: Kirigami.Units.largeSpacing

    signal chosen(string path)

    contentItem: RowLayout {

        spacing: Kirigami.Units.smallSpacing

        QQC2.ToolButton {
            Layout.fillHeight: true

            icon.name: 'mail-attachment'

            text: i18nc("@action:button", "Choose local file")

            onClicked: {
                root.close();
                var fileDialog = openFileDialog.createObject(QQC2.Overlay.overlay) as OpenFileDialog;
                fileDialog.chosen.connect(path => root.chosen(path));
                fileDialog.open();
            }
        }

        Kirigami.Separator {}

        QQC2.ToolButton {
            Layout.fillHeight: true

            icon.name: 'insert-image'
            text: i18nc("@action:button", "Clipboard image")
            onClicked: {
                const path = StandardPaths.standardLocations(StandardPaths.CacheLocation)[0] + "/screenshots/" + (new Date()).getTime() + ".png";
                if (!Clipboard.saveImage(path)) {
                    return;
                }
                root.chosen(path);
                root.close();
            }
        }
    }
    Component {
        id: openFileDialog

        OpenFileDialog {
            parentWindow: Window.window
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        }
    }
}
