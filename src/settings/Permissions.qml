// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

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

    readonly property PowerLevelModel powerLevelModel: PowerLevelModel {
        showMute: false
    }

    readonly property PermissionsModel permissionsModel: PermissionsModel {
        room: root.room
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Privileged Users")
        visible: permissions.count > 0
    }
    FormCard.FormCard {
        visible: permissions.count > 0
        Repeater {
            id: permissions
            model: KSortFilterProxyModel {
                sourceModel: RoomManager.userListModel
                sortRoleName: "powerLevel"
                sortOrder: Qt.DescendingOrder
                filterRowCallback: function (source_row, source_parent) {
                    let powerLevelRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), UserListModel.PowerLevelRole);
                    return powerLevelRole != 0;
                }
            }
            delegate: FormCard.FormTextDelegate {
                id: privilegedUserDelegate
                required property string userId
                required property string name
                required property int powerLevel
                required property string powerLevelString

                text: name
                textItem.textFormat: Text.PlainText
                description: userId
                contentItem.children: RowLayout {
                    spacing: Kirigami.Units.largeSpacing
                    QQC2.Label {
                        id: powerLevelLabel
                        text: privilegedUserDelegate.powerLevelString
                        visible: !root.room.canSendState("m.room.power_levels") || (root.room.memberEffectivePowerLevel(root.room.localMember.id) <= privilegedUserDelegate.powerLevel && privilegedUserDelegate.userId != root.room.localMember.id)
                        color: Kirigami.Theme.disabledTextColor
                    }
                    QQC2.ComboBox {
                        focusPolicy: Qt.NoFocus // provided by parent
                        model: PowerLevelModel {}
                        textRole: "name"
                        valueRole: "value"
                        visible: !powerLevelLabel.visible
                        Component.onCompleted: {
                            let index = indexOfValue(privilegedUserDelegate.powerLevel)
                            if (index === -1) {
                                displayText = privilegedUserDelegate.powerLevelString;
                            } else {
                                currentIndex = index;
                            }
                        }
                        onActivated: {
                            root.room.setUserPowerLevel(privilegedUserDelegate.userId, currentValue);
                        }
                    }
                }
            }
        }
        FormCard.FormDelegateSeparator {
            below: userListSearchCard
        }
        FormCard.AbstractFormDelegate {
            id: userListSearchCard
            visible: root.room.canSendState("m.room.power_levels")

            contentItem: Kirigami.SearchField {
                id: userListSearchField

                autoAccept: false

                Layout.fillWidth: true

                Keys.onUpPressed: userListView.decrementCurrentIndex()
                Keys.onDownPressed: userListView.incrementCurrentIndex()

                onAccepted: (userListView.itemAtIndex(userListView.currentIndex) as Delegates.RoundedItemDelegate).action.trigger()
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

                    radius: Kirigami.Units.cornerRadius
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
                            sourceModel: RoomManager.userListModel
                            filterText: userListSearchField.text

                            onFilterTextChanged: {
                                if (filterText.length > 0 && !userListSearchPopup.visible) {
                                    userListSearchPopup.open();
                                } else if (filterText.length <= 0 && userListSearchPopup.visible) {
                                    userListSearchPopup.close();
                                }
                            }
                        }

                        delegate: Delegates.RoundedItemDelegate {
                            id: userListItem

                            required property string userId
                            required property url avatar
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

                            onClicked: {
                                userListSearchPopup.close();
                                (powerLevelDialog.createObject(root.QQC2.Overlay.overlay, {
                                    room: root.room,
                                    userId: userListItem.userId,
                                    powerLevel: userListItem.powerLevel
                                }) as PowerLevelDialog).open();
                            }

                            Component {
                                id: powerLevelDialog
                                PowerLevelDialog {}
                            }
                        }
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        visible: root.room.canSendState("m.room.power_levels")
        title: i18nc("@title", "Default permissions")
    }
    FormCard.FormCard {
        visible: root.room.canSendState("m.room.power_levels")
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.permissionsModel
                filterRowCallback: function (source_row, source_parent) {
                    return sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsDefaultValueRole);
                }
            }
            delegate: FormCard.FormComboBoxDelegate {
                required property string name
                required property string subtitle
                required property string type
                required property int level
                required property string levelName

                text: name
                description: subtitle
                textRole: "name"
                valueRole: "value"
                model: root.powerLevelModel
                Component.onCompleted: {
                    let index = indexOfValue(level)
                    if (index === -1) {
                        displayText = levelName;
                    } else {
                        currentIndex = index;
                    }
                }
                onCurrentValueChanged: if (root.room.canSendState("m.room.power_levels")) {
                    root.permissionsModel.setPowerLevel(type, currentValue);
                }
            }
        }
    }

    FormCard.FormHeader {
        visible: root.room.canSendState("m.room.power_levels")
        title: i18nc("@title", "Basic permissions")
    }
    FormCard.FormCard {
        visible: root.room.canSendState("m.room.power_levels")
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.permissionsModel
                filterRowCallback: function (source_row, source_parent) {
                    return sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsBasicPermissionRole);
                }
            }
            delegate: FormCard.FormComboBoxDelegate {
                required property string name
                required property string subtitle
                required property string type
                required property int level
                required property string levelName

                text: name
                description: subtitle
                textRole: "name"
                valueRole: "value"
                model: root.powerLevelModel
                Component.onCompleted: {
                    let index = indexOfValue(level)
                    if (index === -1) {
                        displayText = levelName;
                    } else {
                        currentIndex = index;
                    }
                }
                onCurrentValueChanged: if (root.room.canSendState("m.room.power_levels")) {
                    root.permissionsModel.setPowerLevel(type, currentValue);
                }
            }
        }
    }

    FormCard.FormHeader {
        visible: root.room.canSendState("m.room.power_levels")
        title: i18nc("@title", "Event permissions")
    }
    FormCard.FormCard {
        visible: root.room.canSendState("m.room.power_levels")
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.permissionsModel
                filterRowCallback: function (source_row, source_parent) {
                    let isBasicPermissionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsBasicPermissionRole);
                    let isDefaultValueRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsDefaultValueRole);
                    return !isBasicPermissionRole && !isDefaultValueRole;
                }
            }
            delegate: FormCard.FormComboBoxDelegate {
                required property string name
                required property string subtitle
                required property string type
                required property int level
                required property string levelName

                text: name
                description: subtitle
                textRole: "name"
                valueRole: "value"
                model: root.powerLevelModel
                Component.onCompleted: {
                    let index = indexOfValue(level)
                    if (index === -1) {
                        displayText = levelName;
                    } else {
                        currentIndex = index;
                    }
                }
                onCurrentValueChanged: if (root.room.canSendState("m.room.power_levels")) {
                    root.permissionsModel.setPowerLevel(type, currentValue);
                }
            }
        }
        FormCard.AbstractFormDelegate {
            Layout.fillWidth: true

            contentItem: RowLayout {
                Kirigami.ActionTextField {
                    id: newEventAddField

                    Layout.fillWidth: true

                    placeholderText: i18nc("@placeholder", "Event Type…")
                    enabled: NotificationsManager.keywordNotificationAction !== PushRuleAction.Unknown

                    rightActions: Kirigami.Action {
                        icon.name: "edit-clear"
                        visible: newEventAddField.text.length > 0
                        onTriggered: {
                            newEventAddField.text = "";
                        }
                    }

                    onAccepted: {
                        root.permissionsModel.setPowerLevel(newEventAddField.text, newEventPowerLevel.currentValue);
                        newEventAddField.text = "";
                    }
                }
                QQC2.ComboBox {
                    id: newEventPowerLevel
                    focusPolicy: Qt.NoFocus // provided by parent
                    model: root.powerLevelModel
                    textRole: "name"
                    valueRole: "value"
                }
                QQC2.Button {
                    id: addButton

                    text: i18nc("@action:button", "Add keyword")
                    Accessible.name: text
                    icon.name: "list-add"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: newEventAddField.text.length > 0

                    onClicked: {
                        root.permissionsModel.setPowerLevel(newEventAddField.text, newEventPowerLevel.currentValue);
                        newEventAddField.text = "";
                    }

                    QQC2.ToolTip {
                        text: addButton.text
                        delay: Kirigami.Units.toolTipDelay
                    }
                }
            }
        }
    }

    Item {
        visible: permissions.count === 0
        Layout.fillWidth: true
        implicitHeight: root.height * 0.9
        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            text: i18nc("@placeholder", "Loading…")
        }
    }
}
