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

    readonly property bool loading: permissions.count === 0 && !root.room.roomCreatorHasUltimatePowerLevel()

    readonly property PowerLevelModel powerLevelModel: PowerLevelModel {
        showMute: false
    }

    readonly property PermissionsModel permissionsModel: PermissionsModel {
        room: root.room
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Power Levels")
    }
    FormCard.FormCard {
        enabled: root.room.canSendState("m.room.power_levels")
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
        title: i18nc("@title", "Messages")
    }
    FormCard.FormCard {
        enabled: root.room.canSendState("m.room.power_levels")
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.permissionsModel
                filterRowCallback: function (source_row, source_parent) {
                    return sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsMessagePermissionRole);
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
        title: i18nc("@title", "Moderation")
    }
    FormCard.FormCard {
        enabled: root.room.canSendState("m.room.power_levels")
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
        title: i18nc("@title", "General")
    }
    FormCard.FormCard {
        enabled: root.room.canSendState("m.room.power_levels")
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.permissionsModel
                filterRowCallback: function (source_row, source_parent) {
                    return sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsGeneralPermissionRole);
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
        title: i18nc("@title", "Other Events")
    }
    FormCard.FormCard {
        enabled: root.room.canSendState("m.room.power_levels")

        Repeater {
            id: otherEventsRepeater

            model: KSortFilterProxyModel {
                sourceModel: root.permissionsModel
                filterRowCallback: function (source_row, source_parent) {
                    let isBasicPermissionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsBasicPermissionRole);
                    let isDefaultValueRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsDefaultValueRole);
                    let isMessagePermissionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsMessagePermissionRole);
                    let isGeneralPermissionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PermissionsModel.IsGeneralPermissionRole);
                    return !isBasicPermissionRole && !isDefaultValueRole && !isMessagePermissionRole && !isGeneralPermissionRole;
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
        FormCard.FormDelegateSeparator {
            below: addNewEventDelegate
            visible: otherEventsRepeater.count > 0
        }
        FormCard.AbstractFormDelegate {
            id: addNewEventDelegate

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
        visible: root.loading
        Layout.fillWidth: true
        implicitHeight: root.height * 0.9
        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            text: i18nc("@placeholder", "Loading…")
        }
    }
}
