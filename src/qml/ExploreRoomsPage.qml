// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat

/**
 * @brief Component for finding rooms for the public list.
 *
 * This component is based on a SearchPage, adding the functionality to select or
 * enter a server in the header, as well as the ability to manually type a room in
 * if the public room search cannot find it.
 *
 * @sa SearchPage
 */
SearchPage {
    id: root

    /**
     * @brief The connection for the current local user.
     */
    required property NeoChatConnection connection

    /**
     * @brief Whether results should only includes spaces.
     */
    property bool showOnlySpaces: false

    /**
     * @brief Signal emitted when a room is selected.
     *
     * The signal contains all the room's info so that it can be acted
     * upon as required, e.g. joining or entering the room or adding the room as
     * the child of a space.
     */
    signal roomSelected(string roomId,
                        string displayName,
                        url avatarUrl,
                        string alias,
                        string topic,
                        int memberCount,
                        bool isJoined)

    title: i18nc("@action:title", "Explore Rooms")

    Component.onCompleted: focusSearch()

    headerTrailing: ServerComboBox {
        id: serverComboBox
        connection: root.connection
    }

    model: PublicRoomListModel {
        id: publicRoomListModel

        connection: root.connection
        server: serverComboBox.server
        showOnlySpaces: root.showOnlySpaces
    }

    modelDelegate: ExplorerDelegate {
        onRoomSelected: (roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
            root.roomSelected(roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined);
            root.closeDialog();
        }
    }

    listHeaderDelegate: Delegates.RoundedItemDelegate {
        onClicked: _private.openManualRoomDialog()

        text: i18n("Enter a room address")
        icon.name: "compass"
        icon.width: Kirigami.Units.gridUnit * 2
        icon.height: Kirigami.Units.gridUnit * 2
    }

    listFooterDelegate: QQC2.ProgressBar {
        width: ListView.view.width
        leftInset: Kirigami.Units.largeSpacing
        rightInset: Kirigami.Units.largeSpacing
        visible: root.count !== 0 && publicRoomListModel.searching
        indeterminate: true
    }

    searchFieldPlaceholder: i18n("Find a room...")
    noResultPlaceholderMessage: i18nc("@info:label", "No public rooms found")

    Component {
        id: manualRoomDialog
        ManualRoomDialog {}
    }

    QtObject {
        id: _private
        function openManualRoomDialog() {
            let dialog = manualRoomDialog.createObject(applicationWindow().overlay, {connection: root.connection});
            dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                root.roomSelected(roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined);
                root.closeDialog();
            });
            dialog.open();
        }
    }
}
