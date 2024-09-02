// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

/**
 * @brief The base menu for most message types.
 *
 * This menu supports showing a list of actions to be shown for a particular event
 * delegate in a message timeline. The menu supports both desktop and mobile menus
 * with different visuals appropriate to the platform.
 *
 * The menu supports both a list of main actions and the ability to define sub menus
 * using the nested action parameter.
 *
 * For event types that need alternate actions this class can be used as a base and
 * the actions and nested actions can be overwritten to show the alternate items.
 */
Loader {
    id: root

    /**
     * @brief The current connection for the account accessing the event.
     */
    required property NeoChatConnection connection

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The message author.
     *
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    required property var author

    /**
     * @brief The display text of the message as plain text.
     */
    required property string plainText

    /**
     * @brief The text the user currently has selected.
     */
    property string selectedText: ""

    /**
     * @brief The list of menu item actions that have sub-actions.
     *
     * Each action will be instantiated as a single line that open a sub menu.
     */
    property list<Kirigami.Action> nestedActions

    /**
     * @brief The main list of menu item actions.
     *
     * Each action will be instantiated as a single line in the menu.
     */
    property list<Kirigami.Action> actions

    /**
     * @brief Whether the web search menu should be shown or not.
     */
    property bool enableWebSearch: true

    /**
     * Some common actions shared between menus
     */
    component ViewSourceAction: Kirigami.Action {
        visible: NeoChatConfig.developerTools
        text: i18n("View Source")
        icon.name: "code-context"
        onTriggered: RoomManager.viewEventSource(root.eventId)
    }

    component RemoveMessageAction: Kirigami.Action {
        visible: author.isLocalMember || currentRoom.canSendState("redact")
        text: i18nc("@action:button", "Remove")
        icon.name: "edit-delete-remove"
        icon.color: "red"
        onTriggered: {
            let dialog = applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                title: i18nc("@title:dialog", "Remove Message"),
                placeholder: i18nc("@info:placeholder", "Reason for removing this message"),
                actionText: i18nc("@action:button 'Remove' as in 'Remove this message'", "Remove"),
                icon: "delete"
            }, {
                title: i18nc("@title:dialog", "Remove Message"),
                width: Kirigami.Units.gridUnit * 25
            });
            dialog.accepted.connect(reason => {
                currentRoom.redactEvent(root.eventId, reason);
            });
        }
    }

    component ReplyMessageAction: Kirigami.Action {
        text: i18n("Reply")
        icon.name: "mail-replied-symbolic"
        onTriggered: {
            currentRoom.mainCache.replyId = eventId;
            currentRoom.editCache.editId = "";
            RoomManager.requestFullScreenClose();
        }
    }

    component ReportMessageAction: Kirigami.Action {
        text: i18nc("@action:button 'Report' as in 'Report this event to the administrators'", "Report")
        icon.name: "dialog-warning-symbolic"
        visible: !author.isLocalMember
        onTriggered: {
            let dialog = applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                title: i18nc("@title:dialog", "Report Message"),
                placeholder: i18nc("@info:placeholder", "Reason for reporting this message"),
                icon: "dialog-warning-symbolic",
                actionText: i18nc("@action:button 'Report' as in 'Report this event to the administrators'", "Report")
            }, {
                title: i18nc("@title", "Report Message"),
                width: Kirigami.Units.gridUnit * 25
            });
            dialog.accepted.connect(reason => {
                currentRoom.reportEvent(root.eventId, reason);
            });
        }
    }

    component ShowUserAction: Kirigami.Action {
        text: i18nc("@action:inmenu", "Show User")
        icon.name: "username-copy"
        onTriggered: {
            RoomManager.resolveResource(author.id)
        }
    }

    Component {
        id: regularMenu

        QQC2.Menu {
            id: menu
            Instantiator {
                model: root.nestedActions
                delegate: QQC2.Menu {
                    id: menuItem
                    visible: modelData.visible
                    title: modelData.text
                    icon: modelData.icon

                    Instantiator {
                        model: modelData.children
                        delegate: QQC2.MenuItem {
                            text: modelData.text
                            icon.name: modelData.icon.name
                            onTriggered: modelData.trigger()
                        }
                        onObjectAdded: (index, object) => {
                            menuItem.insertItem(0, object);
                        }
                    }
                }
                onObjectAdded: (index, object) => {
                    object.visible = false;
                    menu.addMenu(object);
                }
            }

            Repeater {
                model: root.actions
                QQC2.MenuItem {
                    visible: modelData.visible
                    action: modelData
                    onClicked: root.item.close()
                }
            }
            QQC2.Menu {
                id: webshortcutmenu
                title: i18n("Search for '%1'", webshortcutmodel.trunkatedSearchText)
                icon.name: "search-symbolic"
                property bool isVisible: webshortcutmodel.enabled && root.enableWebSearch
                Component.onCompleted: {
                    webshortcutmenu.parent.visible = isVisible;
                }
                onIsVisibleChanged: webshortcutmenu.parent.visible = isVisible
                Instantiator {
                    model: WebShortcutModel {
                        id: webshortcutmodel
                        selectedText: root.selectedText.length > 0 ? root.selectedText : root.plainText
                        onOpenUrl: RoomManager.resolveResource(url)
                    }
                    delegate: QQC2.MenuItem {
                        text: model.display
                        icon.name: model.decoration
                        onTriggered: webshortcutmodel.trigger(model.edit)
                    }
                    onObjectAdded: (index, object) => webshortcutmenu.insertItem(0, object)
                }
                QQC2.MenuSeparator {}
                QQC2.MenuItem {
                    text: i18n("Configure Web Shortcuts...")
                    icon.name: "configure"
                    visible: !Controller.isFlatpak
                    onTriggered: webshortcutmodel.configureWebShortcuts()
                }
            }
        }
    }
    Component {
        id: mobileMenu

        Kirigami.OverlayDrawer {
            id: drawer
            height: stackView.implicitHeight
            edge: Qt.BottomEdge
            padding: 0
            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0
            topPadding: 0

            parent: applicationWindow().overlay

            QQC2.StackView {
                id: stackView
                width: parent.width
                implicitHeight: currentItem.implicitHeight

                Component {
                    id: nestedActionsComponent
                    ColumnLayout {
                        id: actionLayout
                        property string title: ""
                        property list<Kirigami.Action> actions
                        width: parent.width
                        spacing: 0
                        RowLayout {
                            QQC2.ToolButton {
                                icon.name: 'draw-arrow-back'
                                onClicked: stackView.pop()
                            }
                            Kirigami.Heading {
                                level: 3
                                Layout.fillWidth: true
                                text: actionLayout.title
                                wrapMode: Text.WordWrap
                            }
                        }
                        Repeater {
                            id: listViewAction
                            model: actionLayout.actions

                            FormCard.FormButtonDelegate {
                                icon.name: modelData.icon.name
                                icon.color: modelData.icon.color ?? undefined
                                enabled: modelData.enabled
                                visible: modelData.visible
                                text: modelData.text
                                onClicked: {
                                    modelData.triggered();
                                    root.item.close();
                                }
                            }
                        }
                    }
                }
                initialItem: ColumnLayout {
                    id: popupContent
                    width: parent.width
                    spacing: 0
                    RowLayout {
                        id: headerLayout
                        Layout.fillWidth: true
                        Layout.margins: Kirigami.Units.largeSpacing
                        spacing: Kirigami.Units.largeSpacing
                        KirigamiComponents.Avatar {
                            id: avatar
                            source: author.avatarSource
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                            Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                            Layout.alignment: Qt.AlignTop
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            Kirigami.Heading {
                                level: 3
                                Layout.fillWidth: true
                                text: currentRoom.htmlSafeMemberName(author.id)
                                wrapMode: Text.WordWrap
                            }
                            QQC2.Label {
                                text: plainText
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap

                                onLinkActivated: RoomManager.resolveResource(link, "join")
                            }
                        }
                    }
                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        spacing: 0
                        Layout.fillWidth: true
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 2.5
                        Repeater {
                            model: ["👍", "👎️", "😄", "🎉", "🚀", "👀"]
                            delegate: QQC2.ItemDelegate {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                contentItem: Kirigami.Heading {
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter

                                    font.family: "emoji"
                                    text: modelData
                                }

                                onClicked: {
                                    currentRoom.toggleReaction(eventId, modelData);
                                    root.item.close();
                                }
                            }
                        }
                    }
                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }
                    Repeater {
                        id: listViewAction
                        model: root.actions

                        FormCard.FormButtonDelegate {
                            icon.name: modelData.icon.name
                            icon.color: modelData.icon.color ?? undefined
                            enabled: modelData.enabled
                            visible: modelData.visible
                            text: modelData.text
                            onClicked: {
                                modelData.triggered();
                                root.item.close();
                            }
                        }
                    }

                    Repeater {
                        model: root.nestedActions

                        FormCard.FormButtonDelegate {
                            action: modelData
                            visible: modelData.visible
                            onClicked: {
                                stackView.push(nestedActionsComponent, {
                                    title: modelData.text,
                                    actions: modelData.children
                                });
                            }
                        }
                    }
                }
            }
        }
    }

    asynchronous: true
    sourceComponent: Kirigami.Settings.isMobile ? mobileMenu : regularMenu

    function open() {
        active = true;
    }

    onStatusChanged: if (status == Loader.Ready) {
        if (Kirigami.Settings.isMobile) {
            item.open();
        } else {
            item.popup();
        }
    }
}
