// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    property NeoChatRoom room

    title: i18nc('@title:window', 'Permissions')
    topPadding: 0
    leftPadding: 0
    rightPadding: 0

    UserListModel {
        id: userListModel
        room: root.room
    }

    ListModel {
        id: powerLevelModel
    }

    ColumnLayout {
        spacing: 0
        MobileForm.FormHeader {
            Layout.fillWidth: true
            title: i18n("Privileged Users")
        }
        MobileForm.FormCard {
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                Repeater {
                    model: KSortFilterProxyModel {
                        sourceModel: userListModel
                        sortRole: "powerLevel"
                        sortOrder: Qt.DescendingOrder
                        filterRowCallback: function(source_row, source_parent) {
                            let powerLevelRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), UserListModel.PowerLevelRole)
                            return powerLevelRole > 0;
                        }
                    }
                    delegate: MobileForm.FormTextDelegate {
                        text: name
                        description: userId
                        contentItem.children: RowLayout {
                            spacing: Kirigami.Units.largeSpacing
                            QQC2.Label {
                                visible: !room.canSendState("m.room.power_levels")
                                text: powerLevelString
                                color: Kirigami.Theme.disabledTextColor
                            }
                            QQC2.ComboBox {
                                focusPolicy: Qt.NoFocus // provided by parent
                                model: powerLevelModel
                                textRole: "text"
                                valueRole: "powerLevel"
                                visible: room.canSendState("m.room.power_levels")
                                Component.onCompleted: {
                                    /**
                                     * This is very silly but the only way to populate the model with
                                     * translated strings. Done here because the model needs to be filled
                                     * before the first delegate sets it's current index.
                                     */
                                    if (powerLevelModel.count == 0) {
                                        powerLevelModel.append({"text":  i18n("Member (0)"), "powerLevel": 0});
                                        powerLevelModel.append({"text":  i18n("Moderator (50)"), "powerLevel": 50});
                                        powerLevelModel.append({"text":  i18n("Admin (100)"), "powerLevel": 100});
                                    }
                                    currentIndex = indexOfValue(powerLevel)
                                }
                                onActivated: {
                                    room.setUserPowerLevel(userId, currentValue)
                                }
                            }
                        }
                    }
                }
                MobileForm.FormDelegateSeparator { below: userListSearchCard }
                MobileForm.AbstractFormDelegate {
                    id: userListSearchCard
                    Layout.fillWidth: true
                    visible: room.canSendState("m.room.power_levels")

                    contentItem: Kirigami.SearchField {
                            id: userListSearchField
                            Layout.fillWidth: true
                            autoAccept: false

                            Keys.onUpPressed: userListView.decrementCurrentIndex()
                            Keys.onDownPressed: userListView.incrementCurrentIndex()

                            onAccepted: {
                                let currentUser = userListView.itemAtIndex(userListView.currentIndex);
                                currentUser.action.trigger();
                            }
                    }
                    QQC2.Popup {
                        id: userListSearchPopup

                        x: userListSearchField.x
                        y: userListSearchField.y - height
                        width: userListSearchField.width
                        height: {
                            let maxHeight = userListSearchField.mapToGlobal(userListSearchField.x, userListSearchField.y).y - Kirigami.Units.largeSpacing * 3;
                            let minHeight = Kirigami.Units.gridUnit * 2 + userListSearchPopup.padding * 2;
                            let filterContentHeight = userListView.contentHeight + userListSearchPopup.padding * 2;

                            return Math.max(Math.min(filterContentHeight, maxHeight), minHeight);
                        }
                        padding: Kirigami.Units.smallSpacing
                        modal: false
                        onClosed: userListSearchField.text = ""

                        background: Kirigami.ShadowedRectangle {
                            radius: 4
                            color: Kirigami.Theme.backgroundColor

                            property color borderColor: Kirigami.Theme.textColor
                            border.color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, 0.3)
                            border.width: 1

                            shadow.xOffset: 0
                            shadow.yOffset: 4
                            shadow.color: Qt.rgba(0, 0, 0, 0.3)
                            shadow.size: 8
                        }

                        contentItem: QQC2.ScrollView {
                            // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
                            QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

                            ListView {
                                id: userListView
                                clip: true

                                model: UserFilterModel {
                                    id: userListFilterModel
                                    sourceModel: userListModel
                                    filterText: userListSearchField.text

                                    onFilterTextChanged: {
                                        if (filterText.length > 0 && !userListSearchPopup.visible) {
                                            userListSearchPopup.open()
                                        } else if (filterText.length <= 0 && userListSearchPopup.visible) {
                                            userListSearchPopup.close()
                                        }
                                    }
                                }

                                delegate: Kirigami.BasicListItem {
                                    id: userListItem

                                    implicitHeight: Kirigami.Units.gridUnit * 2
                                    leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

                                    label: name
                                    labelItem.textFormat: Text.PlainText
                                    subtitle: userId
                                    subtitleItem.textFormat: Text.PlainText

                                    action: Kirigami.Action {
                                        id: editPowerLevelAction
                                        onTriggered: {
                                            userListSearchPopup.close()
                                            let dialog = powerLevelDialog.createObject(applicationWindow().overlay, {
                                                room: root.room,
                                                userId: model.userId,
                                                powerLevel: model.powerLevel
                                            });
                                            dialog.open();
                                        }
                                    }

                                    leading: Kirigami.Avatar {
                                        implicitWidth: height
                                        sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                        sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                        source: avatar ? ("image://mxc/" + avatar) : ""
                                        name: model.userId
                                    }

                                    trailing: QQC2.Label {
                                        visible: powerLevel > 0

                                        text: powerLevelString
                                        color: Kirigami.Theme.disabledTextColor
                                        textFormat: Text.PlainText
                                        wrapMode: Text.NoWrap
                                    }

                                    Component {
                                        id: powerLevelDialog
                                        PowerLevelDialog {
                                            id: powerLevelDialog
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        MobileForm.FormHeader {
            Layout.fillWidth: true
            visible: room.canSendState("m.room.power_levels")
            title: i18n("Default permissions")
        }
        MobileForm.FormCard {
            Layout.fillWidth: true
            visible: room.canSendState("m.room.power_levels")
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Default user power level")
                    description: i18n("This is power level for all new users when joining the room")
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.defaultUserPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.defaultUserPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Default power level to set the room state")
                    description: i18n("This is used for all state events that do not have their own entry here")
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.statePowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.statePowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Default power level to send messages")
                    description: i18n("This is used for all message events that do not have their own entry here")
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.defaultEventPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.defaultEventPowerLevel = currentValue
                }
            }
        }

        MobileForm.FormHeader {
            Layout.fillWidth: true
            visible: room.canSendState("m.room.power_levels")
            title: i18n("Basic permissions")
        }
        MobileForm.FormCard {
            Layout.fillWidth: true
            visible: room.canSendState("m.room.power_levels")
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Invite users")
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.invitePowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.invitePowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Kick users")
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.kickPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.kickPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Ban users")
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.banPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.banPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Remove message sent by other users")
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.redactPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.redactPowerLevel = currentValue
                }
            }
        }

        MobileForm.FormHeader {
            Layout.fillWidth: true
            visible: room.canSendState("m.room.power_levels")
            title: i18n("Event permissions")
        }
        MobileForm.FormCard {
            Layout.fillWidth: true
            visible: room.canSendState("m.room.power_levels")
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Change user permissions")
                    description: "m.room.power_levels"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.powerLevelPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.powerLevelPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Change the room name")
                    description: "m.room.name"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.namePowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.namePowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Change the room avatar")
                    description: "m.room.avatar"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.avatarPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.avatarPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Change the room canonical alias")
                    description: "m.room.canonical_alias"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.canonicalAliasPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.canonicalAliasPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Change the room topic")
                    description: "m.room.topic"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.topicPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.topicPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Enable encryption for the room")
                    description: "m.room.encryption"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.encryptionPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.encryptionPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Change the room history visibility")
                    description: "m.room.history_visibility"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.historyVisibilityPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.historyVisibilityPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Set pinned events")
                    description: "m.room.pinned_events"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.pinnedEventsPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.pinnedEventsPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Upgrade the room")
                    description: "m.room.tombstone"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.tombstonePowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.tombstonePowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Set the room server access control list (ACL)")
                    description: "m.room.server_acl"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.serverAclPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.serverAclPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    visible: room.isSpace
                    text: i18n("Set the children of this space")
                    description: "m.space.child"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.spaceChildPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.spaceChildPowerLevel = currentValue
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Set the parent space of this room")
                    description: "m.space.parent"
                    textRole: "text"
                    valueRole: "powerLevel"
                    model: powerLevelModel
                    Component.onCompleted: currentIndex = indexOfValue(room.spaceChildPowerLevel)
                    onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.spaceParentPowerLevel = currentValue
                }
            }
        }
    }
}
