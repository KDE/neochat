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
import io.github.quotient_im.libquotient

FormCard.FormCardPage {
    id: root

    property NeoChatRoom room

    title: i18nc("@title:window", "Members")

    readonly property bool loading: permissions.count === 0 && !root.room.roomCreatorHasUltimatePowerLevel()

    readonly property PowerLevelModel powerLevelModel: PowerLevelModel {
        showMute: false
    }

    Component {
        id: bannedMembersPage
        MembersList {
            title: i18nc("@title", "Banned Members")
            membership: Quotient.MembershipMask.Ban
            room: root.room
            confirmationTitle: i18nc("@title:dialog", "Unban User")
            confirmationSubtitle: i18nc("@info %1 is a matrix ID", "Do you really want to unban %1?", currentMemberId)
            icon: "checkmark-symbolic"
            actionText: i18nc("@action:button", "Unban…")
            actionConfirmationText: i18nc("@action:button", "Unban")
            actionVisible: root.room.canSendState("ban")

            onActionTaken: memberId => root.room.unban(memberId)
        }
    }

    Component {
        id: invitedMembersPage
        MembersList {
            title: i18nc("@title", "Invited Members")
            membership: Quotient.MembershipMask.Invite
            room: root.room
            confirmationTitle: i18nc("@title:dialog", "Uninvite User")
            confirmationSubtitle: i18nc("@info %1 is a matrix ID", "Do you really want to uninvite %1?", currentMemberId)
            icon: "im-ban-kick-user-symbolic"
            actionText: i18nc("@action:button", "Uninvite…")
            actionConfirmationText: i18nc("@action:button", "Uninvite")
            actionVisible: root.room.canSendState("kick")

            onActionTaken: memberId => root.room.kickMember(memberId, "Revoked invite")
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormButtonDelegate {
            id: bannedMemberDelegate

            icon.name: "im-ban-user-symbolic"
            text: i18nc("@action:button", "Banned Members")
            onClicked: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).layers.push(bannedMembersPage)
        }
        FormCard.FormDelegateSeparator {
            above: bannedMemberDelegate
            below: inviteMemberDelegate
        }
        FormCard.FormButtonDelegate {
            id: inviteMemberDelegate

            icon.name: "list-add-user-symbolic"
            text: i18nc("@action:button", "Invited Members")
            onClicked: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).layers.push(invitedMembersPage)
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Privileged Members")
        visible: !root.loading
    }
    FormCard.FormCard {
        visible: !root.loading

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
                            membership: Quotient.MembershipMask.Join

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

                        QQC2.Label {
                            text: i18nc("@info", "No users found.")
                            visible: userListView.count === 0

                            anchors {
                                left: parent.left
                                leftMargin: Kirigami.Units.mediumSpacing
                                verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }
        FormCard.FormDelegateSeparator {
            above: userListSearchCard
        }
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
                required property bool isCreator

                text: name
                textItem.textFormat: Text.PlainText
                description: userId
                contentItem.children: RowLayout {
                    spacing: Kirigami.Units.largeSpacing
                    QQC2.Label {
                        id: powerLevelLabel
                        text: privilegedUserDelegate.powerLevelString
                        visible: (!root.room.canSendState("m.room.power_levels") || (root.room.memberEffectivePowerLevel(root.room.localMember.id) <= privilegedUserDelegate.powerLevel && privilegedUserDelegate.userId != root.room.localMember.id)) || privilegedUserDelegate.isCreator
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
