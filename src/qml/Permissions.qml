// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as KirigamiComponents
import org.kde.kitemmodels

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property NeoChatRoom room

    title: i18nc('@title:window', 'Permissions')

    property UserListModel userListModel: UserListModel {
        id: userListModel
        room: root.room
    }

    property ListModel powerLevelModel: ListModel {
        id: powerLevelModel
    }

    FormCard.FormHeader {
        title: i18n("Privileged Users")
    }
    FormCard.FormCard {
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: userListModel
                sortRoleName: "powerLevel"
                sortOrder: Qt.DescendingOrder
                filterRowCallback: function(source_row, source_parent) {
                    let powerLevelRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), UserListModel.PowerLevelRole)
                    return powerLevelRole > 0;
                }
            }
            delegate: FormCard.FormTextDelegate {
                text: name
                textItem.textFormat: Text.PlainText
                description: userId
                contentItem.children: RowLayout {
                    spacing: Kirigami.Units.largeSpacing
                    QQC2.Label {
                        id: powerLevelLabel
                        visible: !room.canSendState("m.room.power_levels") || (room.getUserPowerLevel(room.localUser.id) <= model.powerLevel && model.userId != room.localUser.id)
                        text: powerLevelString
                        color: Kirigami.Theme.disabledTextColor
                    }
                    QQC2.ComboBox {
                        focusPolicy: Qt.NoFocus // provided by parent
                        model: powerLevelModel
                        textRole: "text"
                        valueRole: "powerLevel"
                        visible: !powerLevelLabel.visible
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
        FormCard.FormDelegateSeparator { below: userListSearchCard }
        FormCard.AbstractFormDelegate {
            id: userListSearchCard
            visible: room.canSendState("m.room.power_levels")

            contentItem: Kirigami.SearchField {
                id: userListSearchField

                autoAccept: false

                Layout.fillWidth: true

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
                leftPadding: Kirigami.Units.smallSpacing / 2
                rightPadding: Kirigami.Units.smallSpacing / 2
                modal: false
                onClosed: userListSearchField.text = ""

                background: Kirigami.ShadowedRectangle {
                    property color borderColor: Kirigami.Theme.textColor

                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                    Kirigami.Theme.inherit: false

                    radius: 4
                    color: Kirigami.Theme.backgroundColor

                    border {
                        color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, 0.3)
                        width: 1
                    }

                    shadow {
                        xOffset: 0
                        yOffset: 4
                        color: Qt.rgba(0, 0, 0, 0.3)
                        size: 8
                    }
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

                        delegate: Delegates.RoundedItemDelegate {
                            id: userListItem

                            required property string userId
                            required property string avatar
                            required property string name
                            required property int powerLevel
                            required property string powerLevelString

                            text: name

                            contentItem: RowLayout {
                                KirigamiComponents.Avatar {
                                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                                    source: userListItem.avatar
                                    name: userListItem.name
                                }

                                Delegates.SubtitleContentItem {
                                    itemDelegate: userListItem
                                    subtitle: userListItem.userId
                                    labelItem.textFormat: Text.PlainText
                                    subtitleItem.textFormat: Text.PlainText
                                    Layout.fillWidth: true
                                }

                                QQC2.Label {
                                    visible: userListItem.powerLevel > 0

                                    text: userListItem.powerLevelString
                                    color: Kirigami.Theme.disabledTextColor
                                    textFormat: Text.PlainText
                                    wrapMode: Text.NoWrap
                                }
                            }

                            action: Kirigami.Action {
                                id: editPowerLevelAction
                                onTriggered: {
                                    userListSearchPopup.close()
                                    let dialog = powerLevelDialog.createObject(applicationWindow().overlay, {
                                        room: root.room,
                                        userId: userListItem.userId,
                                        powerLevel: userListItem.powerLevel
                                    });
                                    dialog.open();
                                }
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

    FormCard.FormHeader {
        visible: room.canSendState("m.room.power_levels")
        title: i18n("Default permissions")
    }
    FormCard.FormCard {
        visible: room.canSendState("m.room.power_levels")
        FormCard.FormComboBoxDelegate {
            text: i18n("Default user power level")
            description: i18n("This is power level for all new users when joining the room")
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.defaultUserPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.defaultUserPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Default power level to set the room state")
            description: i18n("This is used for all state events that do not have their own entry here")
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.statePowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.statePowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Default power level to send messages")
            description: i18n("This is used for all message events that do not have their own entry here")
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.defaultEventPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.defaultEventPowerLevel = currentValue
        }
    }

    FormCard.FormHeader {
        visible: room.canSendState("m.room.power_levels")
        title: i18n("Basic permissions")
    }
    FormCard.FormCard {
        visible: room.canSendState("m.room.power_levels")
        FormCard.FormComboBoxDelegate {
            text: i18n("Invite users")
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.invitePowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.invitePowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Kick users")
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.kickPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.kickPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Ban users")
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.banPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.banPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Remove message sent by other users")
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.redactPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.redactPowerLevel = currentValue
        }
    }

    FormCard.FormHeader {
        visible: room.canSendState("m.room.power_levels")
        title: i18n("Event permissions")
    }
    FormCard.FormCard {
        visible: room.canSendState("m.room.power_levels")
        FormCard.FormComboBoxDelegate {
            text: i18n("Change user permissions")
            description: "m.room.power_levels"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.powerLevelPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.powerLevelPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Change the room name")
            description: "m.room.name"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.namePowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.namePowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Change the room avatar")
            description: "m.room.avatar"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.avatarPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.avatarPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Change the room canonical alias")
            description: "m.room.canonical_alias"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.canonicalAliasPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.canonicalAliasPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Change the room topic")
            description: "m.room.topic"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.topicPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.topicPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Enable encryption for the room")
            description: "m.room.encryption"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.encryptionPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.encryptionPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Change the room history visibility")
            description: "m.room.history_visibility"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.historyVisibilityPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.historyVisibilityPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Set pinned events")
            description: "m.room.pinned_events"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.pinnedEventsPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.pinnedEventsPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Upgrade the room")
            description: "m.room.tombstone"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.tombstonePowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.tombstonePowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            text: i18n("Set the room server access control list (ACL)")
            description: "m.room.server_acl"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.serverAclPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.serverAclPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
            visible: room.isSpace
            text: i18n("Set the children of this space")
            description: "m.space.child"
            textRole: "text"
            valueRole: "powerLevel"
            model: powerLevelModel
            Component.onCompleted: currentIndex = indexOfValue(room.spaceChildPowerLevel)
            onCurrentValueChanged: if(room.canSendState("m.room.power_levels")) room.spaceChildPowerLevel = currentValue
        }
        FormCard.FormComboBoxDelegate {
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
