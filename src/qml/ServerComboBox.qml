
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

QQC2.ComboBox {
    id: root

    /**
     * @brief The connection for the current local user.
     */
    required property NeoChatConnection connection

    /**
     * @brief The server to get the search results from.
     */
    property string server

    Layout.preferredWidth: Kirigami.Units.gridUnit * 10
    Component.onCompleted: currentIndex = 0

    textRole: "url"
    valueRole: "url"
    model: ServerListModel {
        id: serverListModel
        connection: root.connection
    }

    delegate: Delegates.RoundedItemDelegate {
        id: serverItem

        required property int index
        required property string url
        required property bool isAddServerDelegate
        required property bool isHomeServer
        required property bool isDeletable

        text: isAddServerDelegate ? i18n("Add New Server") : url
        highlighted: index === root.highlightedIndex

        topInset: index === 0 ? Kirigami.Units.smallSpacing : Math.round(Kirigami.Units.smallSpacing / 2)
        bottomInset: index === ListView.view.count - 1 ? Kirigami.Units.smallSpacing : Math.round(Kirigami.Units.smallSpacing / 2)

        onClicked: if (isAddServerDelegate) {
            addServerSheet.parent = QQC2.Overlay.overlay
            addServerSheet.open();
        }

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Delegates.SubtitleContentItem {
                itemDelegate: serverItem
                subtitle: serverItem.isHomeServer ? i18n("Home Server") : ""
                Layout.fillWidth: true
            }

            QQC2.ToolButton {
                visible: serverItem.isAddServerDelegate || serverItem.isDeletable
                icon.name: serverItem.isAddServerDelegate ? "list-add" : "dialog-close"
                text: i18nc("@action:button", "Add new server")
                Accessible.name: text
                display: QQC2.AbstractButton.IconOnly

                onClicked: {
                    if (root.currentIndex === serverItem.index && serverItem.isDeletable) {
                        root.currentIndex = 0;
                        root.server = root.currentValue;
                        root.popup.close();
                    }
                    if (serverItem.isAddServerDelegate) {
                        addServerSheet.parent = QQC2.Overlay.overlay
                        addServerSheet.open();
                        serverItem.clicked();
                    } else {
                        serverListModel.removeServerAtIndex(serverItem.index);
                    }
                }
            }
        }
    }

    onActivated: {
        if (currentIndex !== count - 1) {
            root.server = root.currentValue;
        } else {
            // Make sure to reset the combobox as it will display nothing if the "Add Server" item was selected.
            root.currentIndex = 0;
            root.server = root.currentValue;

            addServerSheet.parent = QQC2.Overlay.overlay
            addServerSheet.open();
        }
    }

    Kirigami.Dialog {
        id: addServerSheet

        width: Math.min(Kirigami.Units.gridUnit * 24, QQC2.ApplicationWindow.window.width)

        title: i18nc("@title:window", "Add server")

        onOpened: if (!serverUrlField.isValidServer && !addServerSheet.opened) {
            root.currentIndex = 0;
            root.server = root.currentValue;
        } else if (addServerSheet.opened) {
            serverUrlField.forceActiveFocus();
        }

        onClosed: if (serverUrlField.length <= 0) {
            root.currentIndex = root.indexOfValue(root.server);
        }

        contentItem: ColumnLayout {
            FormCard.FormTextDelegate {
                text: serverUrlField.length > 0 ? (serverUrlField.acceptableInput ? (serverUrlField.isValidServer ? i18n("Valid server entered") : i18n("This server cannot be resolved or has already been added")) : i18n("The entered text is not a valid url")) : i18n("Enter server url e.g. kde.org")
            }
            FormCard.FormTextFieldDelegate {
                id: serverUrlField

                property bool isValidServer: false

                label: i18n("Server URL")
                onTextChanged: {
                    if (acceptableInput) {
                        serverListModel.checkServer(text);
                    }
                }

                validator: RegularExpressionValidator {
                    regularExpression: /^[a-zA-Z0-9-]{1,61}\.([a-zA-Z]{2,}|[a-zA-Z0-9-]{2,}\.[a-zA-Z]{2,3})$/
                }

                Connections {
                    target: serverListModel
                    function onServerCheckComplete(url, valid) {
                        if (url == serverUrlField.text && valid) {
                            serverUrlField.isValidServer = true;
                        }
                    }
                }
            }
        }

        customFooterActions: Kirigami.Action {
            text: i18nc("@action:button", "Ok")
            enabled: serverUrlField.acceptableInput && serverUrlField.isValidServer
            onTriggered: {
                serverListModel.addServer(serverUrlField.text);
                root.currentIndex = root.indexOfValue(serverUrlField.text);
                root.server = root.currentValue;
                serverUrlField.text = "";
                addServerSheet.close();
            }
        }
    }
}
