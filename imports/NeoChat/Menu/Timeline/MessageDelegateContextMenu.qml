// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Dialog 1.0

Loader {
    id: loadRoot

    required property var author
    required property string message
    required property string eventId
    property string eventType: ""
    property string formattedBody: ""
    required property string source

    property list<Kirigami.Action> actions: [
        Kirigami.Action {
            text: i18n("Edit")
            icon.name: "document-edit"
            onTriggered: ChatBoxHelper.edit(message, formattedBody, eventId);
            visible: eventType.length > 0 && author.id === Controller.activeConnection.localUserId && (eventType === "emote" || eventType === "message")
        },
        Kirigami.Action {
            text: i18n("Reply")
            icon.name: "mail-replied-symbolic"
            onTriggered: ChatBoxHelper.replyToMessage(eventId, message, author);
        },
        Kirigami.Action {
            visible: author.id === currentRoom.localUser.id || currentRoom.canSendState("redact")
            text: i18n("Remove")
            icon.name: "edit-delete-remove"
            icon.color: "red"
            onTriggered: currentRoom.redactEvent(eventId);
        },
        Kirigami.Action {
            text: i18n("Copy")
            icon.name: "edit-copy"
            onTriggered: Clipboard.saveText(message)
        },
        Kirigami.Action {
            text: i18n("View Source")
            icon.name: "code-context"
            onTriggered: {
                messageSourceSheet.createObject(page, {'sourceText': loadRoot.source}).open();
            }
        }
    ]

    Component {
        id: regularMenu

        QQC2.Menu {
            Repeater {
                model: loadRoot.actions
                QQC2.MenuItem {
                    visible: modelData.visible
                    action: modelData
                    onClicked: loadRoot.item.close();
                }
            }
        }
        /*
        Kirigami.OverlaySheet {
            id: root

            parent: applicationWindow().overlay

            leftPadding: 0
            rightPadding: 0

            header: Kirigami.Heading {
                text: i18nc("@title:menu Message detail dialog", "Message detail")
            }

            contentItem: ColumnLayout {
                spacing: 0
                RowLayout {
                    id: headerLayout
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.largeSpacing
                    spacing: Kirigami.Units.largeSpacing
                    Kirigami.Avatar {
                        id: avatar
                        source: author.avatarMediaId ? ("image://mxc/" + author.avatarMediaId) : ""
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                        Layout.alignment: Qt.AlignTop
                        name: author.displayName
                        color: author.color
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Kirigami.Heading {
                            level: 3
                            Layout.fillWidth: true
                            text: author.displayName
                            wrapMode: Text.WordWrap
                        }
                        QQC2.Label {
                            text: message
                            Layout.fillWidth: true
                            Layout.maximumWidth: Kirigami.Units.gridUnit * 24
                            wrapMode: Text.WordWrap

                            onLinkActivated: RoomManager.openResource(link);
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

                            contentItem: QQC2.Label {
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter

                                font.pixelSize: 16
                                font.family: "emoji"
                                text: modelData

                            }

                            onClicked: {
                                currentRoom.toggleReaction(eventId, modelData)
                                loadRoot.item.close();
                            }
                        }
                    }
                }
                Kirigami.Separator {
                    Layout.fillWidth: true
                }
            }
        }*/
    }
    Component {
        id: mobileMenu

        Kirigami.OverlayDrawer {
            id: drawer
            height: popupContent.implicitHeight
            edge: Qt.BottomEdge
            padding: 0
            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0
            topPadding: 0

            parent: applicationWindow().overlay

            ColumnLayout {
                id: popupContent
                width: parent.width
                spacing: 0
                RowLayout {
                    id: headerLayout
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.largeSpacing
                    spacing: Kirigami.Units.largeSpacing
                    Kirigami.Avatar {
                        id: avatar
                        source: author.avatarMediaId ? ("image://mxc/" + author.avatarMediaId) : ""
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                        Layout.alignment: Qt.AlignTop
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Kirigami.Heading {
                            level: 3
                            Layout.fillWidth: true
                            text: author.displayName
                            wrapMode: Text.WordWrap
                        }
                        QQC2.Label {
                            text: message
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap

                            onLinkActivated: RoomManager.openResource(link);
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
                                loadRoot.item.close();
                            }
                        }
                    }
                }
                Kirigami.Separator {
                    Layout.fillWidth: true
                }
                Repeater {
                    id: listViewAction
                    model: loadRoot.actions

                    Kirigami.BasicListItem {
                        icon: modelData.icon.name
                        iconColor: modelData.icon.color ?? undefined
                        enabled: modelData.enabled
                        visible: modelData.visible
                        text: modelData.text
                        onClicked: {
                            modelData.triggered()
                            loadRoot.item.close();
                        }
                        implicitHeight: visible ? Kirigami.Units.gridUnit * 3 : 0
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

